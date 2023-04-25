# Cheatsheet:
#   docker volume create customvfs-test
#   docker build -t customvfs .
#   docker run --name customvfs-container --rm -it --privileged -v customvfs-test:/mnt/test customvfs
#   docker run --rm -it --privileged --volumes-from customvfs-container ubuntu:20.04 bash

# Use a base image with the necessary tools
FROM ubuntu:20.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    clang \
    cmake \
    libfuse-dev \
    libgtest-dev \
    libsodium-dev \
    pkg-config \
    libboost-program-options-dev

# Create a working directory for your project
WORKDIR /app

# Copy your project files into the container
COPY . .

# Build the project
RUN mkdir build && cd build && cmake ../ && make

# Set the entry point to the customvfs_exec executable
ENTRYPOINT ["/app/build/customvfs_exec"]

# Set the default command to run your application in test mode with the desired mount point
CMD ["--test", "-m", "/mnt/test", "-f", "-o nonempty -f"]
