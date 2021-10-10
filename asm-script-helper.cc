#include "asm-script-helper.h"
#include <functional>
#include <stdio.h>
#include <sys/stat.h>
#include <exception>
#include <stdexcept>
#include <vector>
#include <iconv.h>


WasmScriptRuntime::WasmScriptRuntime(): mod(nullptr), memory(nullptr) {
    wasm_config_t* config = wasm_config_new();
    wasm_config_set_engine(config, JIT);
    wasm_config_set_compiler(config, CRANELIFT);

    // my build doesn't have this
    // wasmer_features_t* features = wasmer_features_new();
    // wasmer_features_multi_value(features, TRUE);
    // wasmer_config_set_features(config, features);
    // wasmer_features_delete(features);

    engine = wasm_engine_new_with_config(config);
    store = wasm_store_new(engine);
}

WasmScriptRuntime::~WasmScriptRuntime() 
{
    // wasm_module_delete(mod);
    // wasm_instance_delete(instance);
    wasm_store_delete(store);
    wasm_engine_delete(engine);
}

void WasmScriptRuntime::load_script(const char *file_name)
{
    if(mod){
        throw new std::runtime_error("module already loaded");
    }
    FILE *f = fopen(file_name, "r");
    if(!f) {
        throw new std::runtime_error("failed to open script file");
    }
    struct stat file_s;
    stat(file_name, &file_s);

    long len = file_s.st_size;
    char *data = (char*)malloc(len);
    if(fread(data, len, 1, f) != 1) {
        throw new std::runtime_error("failed to read module data");
    }
    fclose(f);

    wasm_byte_vec_t wasm_data;
    wasm_byte_vec_new(&wasm_data, len, data);

    mod = wasm_module_new(store, &wasm_data);

    //we're done with the vector, it can be deleted
    wasm_byte_vec_delete(&wasm_data);
    free(data);

    if(!mod) {
        throw new std::runtime_error("failed to load script");
    }
}

void WasmScriptRuntime::register_fun(const std::string &name, const std::string &sig, std::function<wasm_trap_t* (const wasm_val_vec_t*, wasm_val_vec_t*)> &&fun)
{
    registered_functions.emplace(std::make_pair(name, registered_function(sig, std::move(fun))));
}

wasm_valtype_t* get_type(char s) {
    switch(s) {
    case 'i':
    case 'I':
        return wasm_valtype_new_i32();
    case 'f':
    case 'F':
        return wasm_valtype_new_f32();
    default:
        printf("unknown signature char %c\n", s);
        throw new std::runtime_error("Invalid registered function signature");
    }
}

wasm_functype_t* str_to_fun(const std::string &sig) {
    wasm_valtype_t* rs[1];
    wasm_valtype_vec_t results;
    if(sig[0] == 'V') {
        wasm_valtype_vec_new_empty(&results);
    } else {
        rs[0] = get_type(sig[0]);
        wasm_valtype_vec_new(&results, 1, rs);
    }

    wasm_valtype_vec_t params;
    if(sig.size() == 1) {
        wasm_valtype_vec_new_empty(&params);
    } else {
        std::vector<wasm_valtype_t*> args;
        for (int i = 1; i < sig.size(); ++i) {
            args.push_back(get_type(sig[i]));
        }
        wasm_valtype_vec_new(&params, args.size(), &args[0]);
    }
    
    return wasm_functype_new(&params, &results);
}

wasm_trap_t* WasmScriptRuntime::adapter_fun(void* env, const wasm_val_vec_t* args, wasm_val_vec_t* results)
{
    auto fun = (std::function<wasm_trap_t* (const wasm_val_vec_t*, wasm_val_vec_t*)>*)env;
    return (*fun)(args, results);
}


void WasmScriptRuntime::instantiate() {
    //FIXME we leak if anything throws
    std::vector<wasm_extern_t*> imported_funcs;

    wasm_importtype_vec_t imports_spec;
    wasm_module_imports(mod, &imports_spec);
    if(imports_spec.size == 0) {
        printf("got no imports\n");
    }

    for(int i = 0; i < imports_spec.size; ++i) {
        wasm_importtype_t *imp = imports_spec.data[i];
        const wasm_name_t *impl_mod =  wasm_importtype_module(imp);
        const wasm_name_t *impl_name =  wasm_importtype_name(imp);

        std::string mod_name(impl_mod->data, impl_mod->data + impl_mod->size);
        std::string fun_name(impl_name->data, impl_name->data + impl_name->size);

        wasm_externkind_t kind = wasm_externtype_kind(wasm_importtype_type(imp));

        // printf("\timport [%d] kind:%d module:%s name:%s\n", i, kind, mod_name.c_str(), fun_name.c_str());

        if(kind != WASM_EXTERN_FUNC) {
            throw new std::runtime_error("only function imports are currently supported");
        }

        auto reg = registered_functions.find(mod_name + "::" + fun_name);
        if(reg == registered_functions.end()) {
            printf("could not find %s::%s in registered functions\n", mod_name.c_str(), fun_name.c_str());
            throw new std::runtime_error("missing function from imports");
        }
        auto sig = str_to_fun(reg->second.signature);
        wasm_func_t* wasm_func = wasm_func_new_with_env(store, sig, &WasmScriptRuntime::adapter_fun, &reg->second.fun, nullptr);    
        wasm_functype_delete(sig);
        imported_funcs.push_back(wasm_func_as_extern(wasm_func));
    }

    wasm_extern_vec_t imports = { imported_funcs.size(), &imported_funcs[0] };
    wasm_trap_t *traps = NULL;


    wasm_instance_t *instance = wasm_instance_new(store, mod, &imports, &traps);
    if(!instance) {
        printf("failed to create instance, do we have a trap? %p\n", traps);
        throw new std::runtime_error("failed to instatiate module");
    }

    wasm_extern_vec_t ins_exports;
    wasm_instance_exports(instance, &ins_exports);
    // printf("module has %zu exports\n", ins_exports.size);

    int memory_idx = -1;

    wasm_exporttype_vec_t mod_exports;
    wasm_module_exports(mod, &mod_exports);
    if(mod_exports.size == 0) {
        throw new std::runtime_error("script with no exports, can't do anything with it");
    }
    
    for(int i = 0; i < mod_exports.size; ++i) {
        wasm_exporttype_t *exp = mod_exports.data[i];

        const wasm_externtype_t *type = wasm_exporttype_type(exp);
        const wasm_name_t *name = wasm_exporttype_name(exp);

        std::string fun_name(name->data, name->data + name->size);

        // printf("[%d] have %d with name %s--\n", i, wasm_externtype_kind(type), fun_name.c_str());

        if(wasm_externtype_kind(type) == WASM_EXTERN_MEMORY) {
           memory = wasm_extern_as_memory(ins_exports.data[i]);
        }
        else if(wasm_externtype_kind(type) == WASM_EXTERN_FUNC) {
            exported_functions.emplace(std::make_pair(fun_name, wasm_extern_as_func(ins_exports.data[i])));
        }
    }
    wasm_exporttype_vec_delete(&mod_exports);
    wasm_extern_vec_delete(&ins_exports);
}

wasm_trap_t* WasmScriptRuntime::invoke(const std::string &name, const wasm_val_vec_t& args, wasm_val_vec_t &results)
{
    auto reg = exported_functions.find(name);
    if(reg == exported_functions.end()) {
        throw new std::runtime_error("Could not find exported function " + name);
    }

    return wasm_func_call(reg->second, &args, &results);
}


#define ID_OFFSET -8
#define SIZE_OFFSET -4


#define ARRAYBUFFER_ID 0
#define STRING_ID 1
#define ARRAYBUFFERVIEW_ID = 2;

//FIXME return std::string &&
char* WasmScriptRuntime::get_string(int ptr)
{
    if(!ptr)
        return NULL;
    //tag/size are on negative offsets, so ptr >= -ID_OFFSET
    if (ptr + ID_OFFSET < 0) {
        printf("ptr too small\n");
        return NULL;
    }
    //TODO bounds check
    size_t mem_size = wasm_memory_data_size(memory);
    if (ptr > mem_size) {
        printf("bad string ptr base\n");
        return NULL;
    }

    const char *object_base = wasm_memory_data(memory) + ptr;
    // printf("get_string %d mem: %p\n", ptr, the_memory);
    int type_tag = *(int*)(object_base + ID_OFFSET);
    // printf("type tag is %d\n", type_tag);

    if(type_tag != STRING_ID) {
        printf("bad object tag\n");
        return NULL;
    }
    
    size_t size = (size_t)*(uint32_t*)(object_base + SIZE_OFFSET);

    //FIXME this must be done with overflow checking
    if (ptr + size > mem_size) {
        printf("ptr overflow memory %d %zu -> %zu\n", ptr, size, mem_size);
        return NULL;
    }

    char *base = (char*)malloc(size + 1);

    //this is madening, use eglib code for this instead
    char *input = (char*)object_base;
    size_t input_size = size;
    char *output = base;
    size_t output_size = size;

    iconv_t utf16_to_utf8 = iconv_open("UTF-8", "UTF-16LE");
    size_t res = iconv (utf16_to_utf8, &input, &input_size, &output, &output_size);
    iconv_close (utf16_to_utf8);
    if(input_size == 0) {
        *output = 0;
        return base;
    }

    free(base);
    return NULL;
}