#pragma once

#include <string>

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0

#define HEADER_FRAMES 2 // WATNING!! don't change this value to ensure that header is 100% readable for the decoder
                        // reserving first 2 frames for header is huge waste of space but it is more reliable that way. default frames stores around 300KB (with pixel width of 1, header uses larger pixel width)of data while header needs around 37 bytes (overkill just to be sure)
#define HEADER_PIXEL_SIZE 5

// look for comments in .cpp file
namespace settings
{
    // Global settings
    extern std::string ffmpeg_path;
    extern std::string ffmpeg_additional_args;
    extern std::string input_file_path;
    extern std::string output_file_path;

    extern bool verbose;
    extern bool skip_ffmpeg_check;
    extern bool debug;

    namespace video
    {
        extern unsigned int width;
        extern unsigned int height;
        extern unsigned int fps;
        extern unsigned int pixel_size;
        extern unsigned int color_space;

        extern bool use_color;
    }
}
