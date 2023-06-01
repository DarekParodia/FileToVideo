#!/bin/bash
# Build script for the project
echo "Building the project..."
cmake --build build --target all -j 14 --
echo "Build completed."