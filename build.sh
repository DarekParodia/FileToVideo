#!/bin/bash

# delete old build dir for clean build
if [ -d "build" ]; then
  rm -rf build
fi
mkdir build

# configure cmake
cd build
cmake ..
if [ $? -ne 0 ]; then
    echo "CMake configuration failed. Exiting..."
    exit 1
fi

# build
make -j$(nproc)
if [ $? -ne 0 ]; then
    echo "Make failed. Exiting..."
    exit 1
fi