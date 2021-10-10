# clang driver.c  -I ~/.wasmer/include/ ~/.wasmer/lib/libwasmer.a  && ./a.out
# sh wasm.sh
clang -std=c++11 driver.cc asm-script-helper.cc -I ~/.wasmer/include/ -L ~/.wasmer/lib/ -lwasmer -liconv -lstdc++ && LD_LIBRARY_PATH=~/.wasmer/lib/ ./a.out
