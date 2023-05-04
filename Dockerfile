# Debug mode:
#   docker volume create customvfs-volume
#   docker build -t customvfs .
#   docker run --name customvfs-container --rm -it --privileged -v customvfs-volume:/mnt/fuse customvfs -f "-o nonempty -d -f"
#   docker run --rm -it --privileged --volumes-from customvfs-container ubuntu:20.04 bash

# Release mode:
#   docker build -t customvfs .
#   docker run --name customvfs-container --rm -it --privileged -v <folder to mount>:/mnt/fuse customvfs

FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies and clean up the apt cache
RUN apt-get update && \
    apt-get install -y \
        sudo clang cmake make g++ \
        fuse libfuse-dev libgtest-dev libsodium-dev \
        pkg-config libboost-program-options-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN mkdir build && cd build && cmake ../ && make

ENTRYPOINT ["sudo", "/app/build/customvfs_exec", "-m", "/mnt/fuse/vfs", "-b", "/mnt/fuse/data"]
