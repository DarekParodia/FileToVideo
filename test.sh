#!/bin/bash

# run build script
/bin/bash build.sh
if [ $? -ne 0 ]; then
    echo "Build failed."
    exit 1
fi
printf "\n"

# run program with input arguments
./build/filetovideo $@