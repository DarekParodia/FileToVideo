#pragma once

#include <string>

namespace settings
{
    // Global settings
    extern std::string ffmpeg_path;
    extern std::string input_file_path;
    extern std::string output_file_path;

    extern int width;
    extern int height;
    extern int fps;

    extern bool verbose;
    extern bool skip_ffmpeg_check;
    extern bool debug;
}
