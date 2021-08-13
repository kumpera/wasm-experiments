# clang driver.c  -I ~/.wasmer/include/ ~/.wasmer/lib/libwasmer.a  && ./a.out
# sh wasm.sh
clang driver.c  -I ~/.wasmer/include/ -L ~/.wasmer/lib/ -lwasmer && LD_LIBRARY_PATH=~/.wasmer/lib/ ./a.out