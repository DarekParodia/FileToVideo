#include "settings.h"

// default settings
namespace settings
{
    std::string ffmpeg_path = "ffmpeg";
    std::string ffmpeg_additional_args = "";
    std::string input_file_path = "";
    std::string output_file_path = "";

    bool verbose = true;
    bool skip_ffmpeg_check = false;
    bool debug = false;

    namespace video
    {
        unsigned int width = 640;
        unsigned int height = 480;
        unsigned int fps = 30;
        unsigned int pixel_size = 2;  // just artificial width and height of pixel for reliability with compression // for now make sure thath width and height are divisible by pixel size
        unsigned int color_space = 1; // how much diffrent colors per color channel (1-255) WARNING: setting this too high can resault in compression changing the colors and making the video unreadable for decoding  // not implemented yet

        bool use_color = false; // set to true to use 3 times more data per pixel (rgb yk, no alpha because streaming platforms don't support it anyway)
    }
}