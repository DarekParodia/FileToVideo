#include "settings.h"

// default settings
namespace settings
{
    std::string ffmpeg_path = "ffmpeg";
    std::string input_file_path = "";
    std::string output_file_path = "";

    int width = 640;
    int height = 480;
    int fps = 30;

    bool verbose = true;
    bool skip_ffmpeg_check = false;
    bool debug = false;
}