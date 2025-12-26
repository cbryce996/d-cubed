FROM ubuntu:25.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    wget \
    clang-format \
    clang-tidy \
    cppcheck \
    lcov \
    pkg-config \
    libsdl3-dev \
    libglm-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . /app

RUN mkdir -p /app/build

RUN chmod +x /app/scripts/*.sh

CMD ["/bin/bash"]
