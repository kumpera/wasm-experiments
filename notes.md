## if you want to update your env vars to get wasmer into it
source ~/.wasienv/wasienv.sh

## building
docker build -t wasmy:dev .


## running

docker run --rm -it -v $(pwd):/project wasmy:dev bash

### In case you don't want to deal with building the whole world to get the AssemblyScript example built, do the following:

```
mkdir asm-script/build
cp optimized.wasm asm-script/build
``` 

From the shell, run `make run`

## TODO

The current API is really really bad!
We should lock the wasmer version in the docker image since they constantly break it.
Bring VW into to simplify the work