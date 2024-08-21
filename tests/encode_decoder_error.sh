#!/bin/bash

executable_name="filetovideo"
tests_dir=$(dirname $(realpath $0))

# Search for the executable in common locations
if [ -z "$executable_path" ]; then
    echo "Failure: filetovideo not found."
    exit 1
else
    echo "filetovideo found at $executable_path"
fi

# Test for encoding error
echo "Testing for encoding error..."
$executable_path -i tests/data/encode_error.txt -o tests/data/encode_error.mp4
