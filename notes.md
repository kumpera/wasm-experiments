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


## Running the read/write feature:

From `/project/external/vowpal_wabbit/build/vowpalwabbit`
Do: `./vw -d tst.dat  --wasm --script /project/asm-script/build/optimized.wasm`

`tst.dat` is the first line of `train-sets/0002.dat`:

```
0.521144 1 PFF/20091028|T PFF |f t1:-0.0236849 t5:-0.10215 r5:0.727735 t10:-0.0387662 r10:0.911208 t20:-0.00777943 r20:0.952668 t40:0.014542 r40:0.832479 t60:0.00395449 r60:0.724504 t90:0.0281418 r90:0.784653
```

And `optimized.wasm` was rebuilt to read/write its features.

## TODO

- The current API is really really bad!
- We should lock the wasmer version in the docker image since they constantly break it.
- Bring VW into to simplify the work
