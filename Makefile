COMPILER = g++
LD = ld

all: wasmy

driver.o: driver.cc asm-script-helper.h
	$(COMPILER) -g -std=c++11  -I ~/.wasmer/include/ -L ~/.wasmer/lib/ driver.cc -c -o driver.o

asm-script-helper.o: asm-script-helper.cc asm-script-helper.h
	$(COMPILER) -g -std=c++11  -I ~/.wasmer/include/ -L ~/.wasmer/lib/ asm-script-helper.cc -c -o asm-script-helper.o

wasmy: driver.o asm-script-helper.o
	$(COMPILER) -g -L ~/.wasmer/lib/ -L  /usr/lib/x86_64-linux-gnu/ driver.o asm-script-helper.o -lwasmer -lstdc++ -o wasmy

clean:
	rm -f driver.o asm-script-helper.o wasmy

run: wasmy
	LD_LIBRARY_PATH=~/.wasmer/lib/  ./wasmy

.PHONY: clean run
