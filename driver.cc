#include <sys/stat.h>
#include <stdio.h>
#include "asm-script-helper.h"

static const char *file_name = "asm-script/build/optimized.wasm";

class WasmBindings : public WasmScriptRuntime {
    wasm_trap_t* read_feature(const wasm_val_vec_t* args, wasm_val_vec_t* results) {
        //lets extract those strings now
        char *ns = get_string(args->data[0].of.i32);
        char *name = get_string(args->data[1].of.i32);
        printf("read_feature %s::%s\n", ns, name);
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
        printf("Abort called\n"); //FIXME should return a trap
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