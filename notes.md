# Wasm-Experiments

If you want to update your env vars to get wasmer into it:

```sh
source ~/.wasienv/wasienv.sh
```

## Building

### Prepare docker image

```sh
docker build -t wasmy:dev .
```

### Build driver
```sh
docker run --rm -it -v $(pwd):/project wasmy:dev "make all"
```

### Build assemblyscript

```
docker run --rm -it -v $(pwd):/project wasmy:dev "cd asm-script && npm install && npm run build"
```

## Running
```sh
docker run --rm -it -v $(pwd):/project wasmy:dev "make run"
```

## TODO

- The current API is really really bad!
- We should lock the wasmer version in the docker image since they constantly break it.
- Bring VW into to simplify the work
