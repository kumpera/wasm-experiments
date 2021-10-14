# FROM alpine:latest
FROM ubuntu:20.04

VOLUME "/project"
WORKDIR "/project"


ENV LC_ALL C.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt install --yes --no-install-recommends \
        ca-certificates \
        build-essential cmake g++ make wget ninja-build curl valgrind gdb \
        git python3 python3-pip \
        nodejs npm \
        # VW Test deps
        libboost-test-dev netcat git python3 python3-pip \
        # VW Boost deps
        libboost-dev libboost-program-options-dev libboost-math-dev libboost-system-dev \
        # VW Other deps
        libflatbuffers-dev zlib1g-dev \
        # Clang tools
        clang-format clang-tidy \
        sudo \
        && apt-get clean autoclean \
        && apt-get autoremove -y \
        && rm -rf /var/lib/{apt,dpkg,cache,log}

RUN curl https://get.wasmer.io -sSfL | sh

ENTRYPOINT [ "bash", "-c" ]