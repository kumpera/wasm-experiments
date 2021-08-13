#include <sys/stat.h>
#include <stdio.h>


#include "wasmer_wasm.h"
// #include "wasm.h"
// #include "wasmer.h"
static wasm_memory_t *the_memory;

static const char *file_name = "asm-script/build/optimized.wasm";

static char* kind_to_name[] = {
  "WASM_EXTERN_FUNC",
  "WASM_EXTERN_GLOBAL",
  "WASM_EXTERN_TABLE",
  "WASM_EXTERN_MEMORY",
};

wasm_trap_t* func_adapter_read_feature(const wasm_val_vec_t* args, wasm_val_vec_t* results)
{
    //lets extract those strings now
    printf("read_feature\n");
    wasm_val_t val = WASM_F32_VAL(1);
    wasm_val_copy(&results->data[0], &val);

    return NULL;
}

wasm_trap_t* func_adapter_write_feature(const wasm_val_vec_t* args, wasm_val_vec_t* results)
{
    printf("write_feature\n");
    return NULL;
}

wasm_trap_t* func_adapter_abort(const wasm_val_vec_t* args, wasm_val_vec_t* results)
{
    printf("abort!\n");
    return NULL;
}

static wasm_functype_t* wasm_functype_new_4_0(
  wasm_valtype_t* p1, wasm_valtype_t* p2, wasm_valtype_t* p3, wasm_valtype_t* p4) {
  wasm_valtype_t* ps[4] = {p1, p2, p3, p4};
  wasm_valtype_vec_t params, results;
  wasm_valtype_vec_new(&params, 4, ps);
  wasm_valtype_vec_new_empty(&results);
  return wasm_functype_new(&params, &results);
}

int main() {
    FILE *f = fopen(file_name, "r");
    struct stat file_s;
    stat(file_name, &file_s);

    long len = file_s.st_size;
    char *data = malloc(len);
    if(fread(data, len, 1, f) != 1) {
        printf("failed to read input\n");
        return 1;
    }
    fclose(f);

    wasm_byte_vec_t wasm_data;
    wasm_byte_vec_new(&wasm_data, len, data);

    wasm_config_t* config = wasm_config_new();
    wasm_config_set_engine(config, JIT);
    wasm_config_set_compiler(config, CRANELIFT);

    // my build doesn't have this
    // wasmer_features_t* features = wasmer_features_new();
    // wasmer_features_multi_value(features, TRUE);
    // wasmer_config_set_features(config, features);
    // wasmer_features_delete(features);

    //what's an engine
    wasm_engine_t* engine = wasm_engine_new_with_config(config);
    // wasm_config_delete(config); < can't delete while in use, aparently (pass ownership?)

    wasm_store_t* store = wasm_store_new(engine);

    wasm_module_t *mod = wasm_module_new(store, &wasm_data);
    if(!mod) {
        printf("compilation failed, bye!\n");
        return 1;
    }
    //we're done with the vector, it can be deleted
    wasm_byte_vec_delete(&wasm_data);
    free(data);


    printf("module compiled and ready, but lets figure out exports\n");
    int __process_example_idx = -1;
    // int __new_idx = -1;
    // int __collect = -1;

    wasm_exporttype_vec_t vec;
    wasm_module_exports(mod, &vec);
    if(vec.size == 0) {
        printf("got no exports\n");
        return 1;
    }    

    printf("got %zu exports\n", vec.size);
    char buffer[1024];
    for(int i = 0; i <vec.size; ++i) {
        wasm_exporttype_t *exp = vec.data[i];

        const wasm_externtype_t *type = wasm_exporttype_type(exp);
        const wasm_name_t *name = wasm_exporttype_name(exp);

        memcpy(buffer, name->data, name->size);
        buffer[name->size] = 0;
        
        printf("[%d] kind %d(%s) name %s\n", i, 
        wasm_externtype_kind(type), 
        kind_to_name[wasm_externtype_kind(type)], 
        buffer);

        if(wasm_externtype_kind(type) == WASM_EXTERN_FUNC) {
            if (!strncmp("process_example", name->data, name->size)) {
                printf("found our puppy at %d\n", i);
                __process_example_idx = i;
            }
        }
    }
    wasm_exporttype_vec_delete(&vec);

    if(__process_example_idx == -1) {
        printf("didn't find export we want\n");
        return 1;
    }

    //figure out imports
    wasm_importtype_vec_t imports_spec;
    wasm_module_imports(mod, &imports_spec);
    if(imports_spec.size == 0) {
        printf("got no imports\n");
        return 1;
    }

    for(int i = 0; i < imports_spec.size; ++i) {
        wasm_importtype_t *imp = imports_spec.data[i];
        const wasm_name_t *impl_mod =  wasm_importtype_module(imp);
        const wasm_name_t *impl_name =  wasm_importtype_name(imp);


        memcpy(buffer, impl_name->data, impl_name->size);
        buffer[impl_name->size] = 0;

        printf(" import [%d] module:%s name:%s\n", i, impl_mod->data, buffer);
    }

    //not gonna do proper linking, just inject our export
    wasm_functype_t* read_sig = wasm_functype_new_2_1(
        wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_f32());
    wasm_functype_t* write_sig = wasm_functype_new_3_0(
        wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_f32());
    wasm_functype_t* abort_sig = wasm_functype_new_4_0(
        wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());

    wasm_func_t* export_1_func = wasm_func_new(store, read_sig, func_adapter_read_feature);
    wasm_func_t* export_2_func = wasm_func_new(store, write_sig, func_adapter_write_feature);
    wasm_func_t* export_3_func = wasm_func_new(store, abort_sig, func_adapter_abort);
    wasm_functype_delete(read_sig);
    wasm_functype_delete(write_sig);
    wasm_functype_delete(abort_sig);

    wasm_extern_t* externs[] = {
        wasm_func_as_extern(export_1_func),
        wasm_func_as_extern(export_2_func),
        wasm_func_as_extern(export_3_func),
    };


    //TODO if we expose an API, we got to figure out how to resolve it.
    wasm_extern_vec_t imports = WASM_ARRAY_VEC(externs);
    wasm_trap_t *traps = NULL;

    wasm_instance_t *instance = wasm_instance_new(store, mod, &imports, &traps);
    if(!instance) {
        printf("failed to create instance, do we have a trap? %p\n", traps);
        return 1;
    }

    wasm_extern_vec_t exports;
    wasm_instance_exports(instance, &exports);
    printf("module has %zu exports\n", exports.size);

    if (exports.size == 0) {
        return 1;
    }
    
    the_memory = wasm_extern_as_memory(exports.data[0]);

    wasm_func_t* func = wasm_extern_as_func(exports.data[__process_example_idx]);
    wasm_extern_vec_delete(&exports);

    wasm_val_vec_t arguments_as_array;
    wasm_val_vec_t results_as_array;
    wasm_val_vec_new_empty(&arguments_as_array);
    wasm_val_vec_new_empty(&results_as_array);

    wasm_trap_t* trap = wasm_func_call(func, &arguments_as_array, &results_as_array);
    if(trap) {
        wasm_message_t msg;
        wasm_trap_message(trap, &msg);
        printf("func call failed! %s\n", msg.data);
        wasm_byte_vec_delete(&msg);
        
        return 1;
    }

    // printf("the result is %d\n", results[0].of.i32);

    printf("done \n");

    wasm_module_delete(mod);
    wasm_instance_delete(instance);
    wasm_store_delete(store);
    wasm_engine_delete(engine);

    return 0;
}