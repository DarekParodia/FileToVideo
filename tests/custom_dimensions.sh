#!/bin/bash

executable_name=$1
tmp_dir="/tmp/filetovideo/tests/"
tmp_filename="custom_dimensions.mp4"

# Search for the executable in common locations
if [ -z "$executable_name" ]; then
    echo "Failure: filetovideo not found."
    exit 1
else
    echo "filetovideo found at $executable_name"
fi

# Check if the input file exists
if [ ! -f $1 ]; then
    echo "Failure: Input file not found."
    exit 1
fi

# Test for encoding error
if [ ! -d $tmp_dir ]; then
    mkdir -p $tmp_dir
fi

echo "Testing encoding..."
$executable_name -i=$2 -o=$tmp_dir/$tmp_filename -y

if [ $? -ne 0 ]; then
    echo "Failure: Encoding error."
    exit 1
fi

# Test for decoding error
echo "Testing decoding..."
$executable_name -i=$tmp_dir/$tmp_filename -o=$tmp_dir/decoded_$tmp_filename -d -y -w=1920 -h=1080

if [ $? -ne 0 ]; then
    echo "Failure: Decoding error."
    exit 1
fi

# Check if the decoded file is the same as the input file
if ! cmp -s $2 $tmp_dir/decoded_$tmp_filename; then
    echo "Failure: Decoded file is different from the input file."
    exit 1
fi

echo "Success: Encoding and decoding successful."
exit 0