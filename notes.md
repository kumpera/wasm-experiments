## if you want to update your env vars to get wasmer into it
source ~/.wasienv/wasienv.sh

## building
docker build -t wasmy:dev .


## running

docker run --rm -it -v $(pwd):/project wasmy:dev bash

From the shell, run `make run`

## TODO

The current API is really really bad!
We should lock the wasmer version in the docker image since they constantly break it.
Bring VW into to simplify the work