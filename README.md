# FileToVideo

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/E1E8P5UQA)

A command-line tool designed to convert various file formats into video presentations. It supports decoding existing videos and generating new ones from images or other multimedia files, providing flexibility for users to create engaging video content from static sources.

Here is an example of a video generated using FileToVideo:

![FileToVideo Previw](./.github/preview.gif)

## Table of Contents

- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Arguments Guide](#arguments-guide)
- [Examples](#examples)
- [Contributing](#contributing)

## Requirements

FileToVideo requires the following dependencies to be installed on your system:

- ``` cmake ```
- ``` gcc ``` or some other C++ compiler that will work
- ``` ffmpeg ```
- ``` youtube-dl ``` (for decoding from youtube) **OPTIONAL**
- ``` libavcodec-dev ```
- ``` libavformat-dev ```
- ``` libavutil-dev ```
- ``` libswscale-dev ```

To install the required dependencies on Ubuntu, run the following commands:

```bash
sudo apt-get install cmake g++ ffmpeg youtube-dl libavcodec-dev libavformat-dev libavutil-dev libswscale-dev
```

(I have not tested this command and if dependencies names are correct but i think it should work) (i use arch btw)

## Installation

To install FileToVideo, ensure you have C++ and CMake installed on your system. Clone this repository and build the project using the provided `build.sh` script.

```bash
git clone https://github.com/DarekParodia/FileToVideo.git
cd FileToVideo
./build.sh
```

## Usage

FileToVideo is operated via command-line arguments. Navigate to the project directory and execute the binary with the desired options.

## Arguments Guide

- `-i, --input-file`: Specifies the input file path. (Required)
- `-o, --output-file`: Defines the output video file path. (Required)
- `-d, --decode`: Decode mode flag. When set, the tool operates in decode mode.
- `-w, --width`: Set the width of the generated video.
- `-h, --height`: Set the height of the generated video.
- `-f, --fps`: Frames per second (FPS) for the generated video.
- `-p, --pixel-size`: Size of a pixel in the generated video.
- `-c, --use-color`: Enables color usage in the video generation process.
- `-y, --no-confirm`: Skips the confirmation prompt before starting the conversion process.
- `--ffmpeg-path`: Path to the FFmpeg executable. (Required unless FFmpeg is in PATH)
- `--ytdl-path`: Path to the youtube-dl executable. (Required unless youtube-dl is in PATH and you're decoding from youtube)

## Examples

```bash
bash ./FileToVideo -i=/path/to/input.jpg -o=/path/to/output.mp4 -w=1920 -h=1080 -f=30
```

### Contributing

All contributions are welcome! Just create a pull request with your changes.
