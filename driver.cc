#include <sys/stat.h>
#include <stdio.h>

#include "wasmer_wasm.h"
#include "asm-script-helper.h"

// static wasm_memory_t *the_memory;


// #define ID_OFFSET -8
// #define SIZE_OFFSET -4


// #define ARRAYBUFFER_ID 0
// #define STRING_ID 1
; // const ARRAYBUFFERVIEW_ID = 2;

// static char* get_string(int ptr)
// {
//     if(!ptr)
//         return NULL;
//     //tag/size are on negative offsets, so ptr >= -ID_OFFSET
//     if (ptr + ID_OFFSET < 0) {
//         printf("ptr too small\n");
//         return NULL;
//     }
//     //TODO bounds check
//     size_t mem_size = wasm_memory_data_size(the_memory);
//     if (ptr > mem_size) {
//         printf("bad string ptr base\n");
//         return NULL;
//     }

//     const char *object_base = wasm_memory_data(the_memory) + ptr;
//     // printf("get_string %d mem: %p\n", ptr, the_memory);
//     int type_tag = *(int*)(object_base + ID_OFFSET);
//     // printf("type tag is %d\n", type_tag);

//     if(type_tag != STRING_ID) {
//         printf("bad object tag\n");
//         return NULL;
//     }
    
//     size_t size = (size_t)*(uint32_t*)(object_base + SIZE_OFFSET);

//     //FIXME this must be done with overflow checking
//     if (ptr + size > mem_size) {
//         printf("ptr overflow memory %d %zu -> %zu\n", ptr, size, mem_size);
//         return NULL;
//     }

//     char *base = (char*)malloc(size + 1);

//     //this is madening, use eglib code for this instead
//     char *input = (char*)object_base;
//     size_t input_size = size;
//     char *output = base;
//     size_t output_size = size;

//     iconv_t utf16_to_utf8 = iconv_open("UTF-8", "UTF-16LE");
//     size_t res = iconv (utf16_to_utf8, &input, &input_size, &output, &output_size);
//     iconv_close (utf16_to_utf8);
//     if(input_size == 0) {
//         *output = 0;
//         return base;
//     }

//     free(base);
//     return NULL;
// }

static const char *file_name = "asm-script/build/optimized.wasm";

// static char* kind_to_name[] = {
//   "WASM_EXTERN_FUNC",
//   "WASM_EXTERN_GLOBAL",
//   "WASM_EXTERN_TABLE",
//   "WASM_EXTERN_MEMORY",
// };

// wasm_trap_t* func_adapter_read_feature(const wasm_val_vec_t* args, wasm_val_vec_t* results)
// {
//     printf("read feature %d %d\n", args->data[0].of.i32, args->data[1].of.i32);
//     //lets extract those strings now
//     // printf("print string %d %s\n", args->data[0].of.i32, wasm_memory_data(the_memory) + args->data[0].of.i32);
//     // char *ns = get_string(args->data[0].of.i32);
//     // char *name = get_string(args->data[1].of.i32);
//     // printf("read_feature %s::%s\n", ns, name);
//     wasm_val_t val = WASM_F32_VAL(1);
//     wasm_val_copy(&results->data[0], &val);
//     // free(ns);
//     // free(name);

//     return NULL;
// }

// wasm_trap_t* func_adapter_write_feature(const wasm_val_vec_t* args, wasm_val_vec_t* results)
// {
//     // char *ns = get_string(args->data[0].of.i32);
//     // char *name = get_string(args->data[1].of.i32);
//     float val = args->data[2].of.f32;

//     printf("write feature %d %d << %f\n", args->data[0].of.i32, args->data[1].of.i32, val);
//     // printf("write_feature %s::%s -> %f\n", ns, name, val);
//     // free(ns);
//     // free(name);

//     return NULL;
// }

// wasm_trap_t* func_adapter_abort(const wasm_val_vec_t* args, wasm_val_vec_t* results)
// {
//     printf("abort!\n");
//     return NULL;
// }

// static wasm_functype_t* wasm_functype_new_4_0(
//   wasm_valtype_t* p1, wasm_valtype_t* p2, wasm_valtype_t* p3, wasm_valtype_t* p4) {
//   wasm_valtype_t* ps[4] = {p1, p2, p3, p4};
//   wasm_valtype_vec_t params, results;
//   wasm_valtype_vec_new(&params, 4, ps);
//   wasm_valtype_vec_new_empty(&results);
//   return wasm_functype_new(&params, &results);
// }

class WasmBindings : public WasmScriptRuntime {
    wasm_trap_t* read_feature(const wasm_val_vec_t* args, wasm_val_vec_t* results) {
        //lets extract those strings now
        char *ns = get_string(args->data[0].of.i32);
        char *name = get_string(args->data[1].of.i32);
        printf("print string %s::%s\n", ns, name);
        wasm_val_t val = WASM_F32_VAL(1);
        wasm_val_copy(&results->data[0], &val);
        free(ns);
        free(name);
        return nullptr;
    }

    wasm_trap_t* write_feature(const wasm_val_vec_t* args, wasm_val_vec_t* results) {     
        char *ns = get_string(args->data[0].of.i32);
        char *name = get_string(args->data[1].of.i32);
        float val = args->data[2].of.f32;

        printf("write_feature %s::%s -> %f\n", ns, name, val);
        free(ns);
        free(name);
        return nullptr;
    }

    wasm_trap_t* abort(const wasm_val_vec_t* args, wasm_val_vec_t* results) {
        printf("ABORT!!!\n"); //FIXME should return a trap
        return nullptr;
    }

public:
    WasmBindings (const std::string &script_name) {
        load_script(script_name.c_str());
        #define REGISTER(name, sig, func) \
            register_fun(name, sig, [this](const wasm_val_vec_t *p, wasm_val_vec_t *r) { return func(p, r); });


        REGISTER("index::vw.read_feature", "FII", read_feature);
        REGISTER("index::vw.write_feature", "VIIF", write_feature);
        REGISTER("env::abort", "VIIII", abort);

        // rt.register_fun("index::vw.read_feature", "FII", func_adapter_read_feature);
        // rt.register_fun("index::vw.write_feature", "VIIF", func_adapter_write_feature);
        // rt.register_fun("env::abort", "VIIII", func_adapter_abort);

    }
};

int main() {
    WasmBindings rt(file_name);
    rt.instantiate();

    wasm_val_vec_t args = WASM_EMPTY_VEC;
    wasm_val_vec_t rets = WASM_EMPTY_VEC;

    wasm_trap_t* trap = rt.invoke("process_example", args, rets);
    if(trap) {
        wasm_message_t msg;
        wasm_trap_message(trap, &msg);
        printf("func call failed! %s\n", msg.data);
        wasm_byte_vec_delete(&msg);
        
        return 1;
    }

    return 0;
}