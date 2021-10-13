#pragma once

#include <string>
#include <unordered_map>
#include <functional>

#include <wasmer.h>

class WasmScriptRuntime
{
    struct registered_function {
        registered_function(const std::string &sig, std::function<wasm_trap_t* (const wasm_val_vec_t*, wasm_val_vec_t*)> &&fun):
            signature(sig),
            fun(std::move(fun)) {}

        std::string signature;
        std::function<wasm_trap_t* (const wasm_val_vec_t*, wasm_val_vec_t*)> fun;
    };
    wasm_engine_t* engine;
    wasm_store_t* store;
    wasm_module_t *mod;
    wasm_instance_t *instance;
    wasm_memory_t *memory;
    wasm_extern_vec_t ins_exports; //must keep it alive or all stuff we peel from it becomes invalid
    std::unordered_map<std::string, wasm_func_t*> exported_functions;
    std::unordered_map<std::string, registered_function> registered_functions;

    static wasm_trap_t* adapter_fun(void* env, const wasm_val_vec_t* args, wasm_val_vec_t* results);
public:
    WasmScriptRuntime();
    ~WasmScriptRuntime();

    void load_script(const char *file_name);
    void register_fun(const std::string &name, const std::string &sig, std::function<wasm_trap_t* (const wasm_val_vec_t*, wasm_val_vec_t*)> &&fun);
    void instantiate();

    wasm_trap_t* invoke(const std::string &name, const wasm_val_vec_t& args, wasm_val_vec_t &results);

    char* get_string(int ptr);

};
