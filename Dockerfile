# FROM alpine:latest
FROM ubuntu:20.04

VOLUME "/project"
WORKDIR "/project"

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt install --yes --no-install-recommends \
        ca-certificates \
        build-essential cmake g++ make wget ninja-build curl valgrind gdb \
        git python3 python3-pip \
        sudo \
        && apt-get clean autoclean \
        && apt-get autoremove -y \
        && rm -rf /var/lib/{apt,dpkg,cache,log}

RUN curl https://get.wasmer.io -sSfL | sh

ENTRYPOINT [ "bash", "-c" ]