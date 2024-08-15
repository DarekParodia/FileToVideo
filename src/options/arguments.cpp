#include "arguments.h"

namespace arguments
{
    //
    // ================ input_file ================
    //

    input_file::input_file()
    {
        name = "input_file";
        description = "The input file to convert to a video";
        shortcall = "-i";
        longcall = "--input-file";
        required = true;
    }

    input_file::~input_file()
    {
    }

    void input_file::execute(std::string arg)
    {
        logger.debug("Executing input_file option with argument: " + arg);

        std::string value = get_arg_value(arg);

        if (value.empty())
        {
            logger.error("No value specified for input_file option");
            exit(1);
        }

        settings::input_file_path = value;
        this->executed = true;
    }

    void input_file::check()
    {
        if (settings::input_file_path.empty())
        {
            logger.error("No input file specified");
            exit(1);
        }
    }

    //
    // ================ output_file ================
    //

    output_file::output_file()
    {
        name = "output_file";
        description = "The output file to save the video to";
        shortcall = "-o";
        longcall = "--output-file";
        required = true;
    }

    output_file::~output_file()
    {
    }

    void output_file::execute(std::string arg)
    {
        logger.debug("Executing output_file option with argument: " + arg);

        std::string value = get_arg_value(arg);

        if (value.empty())
        {
            logger.error("No value specified for output_file option");
            exit(1);
        }

        settings::output_file_path = value;
        this->executed = true;
    }

    void output_file::check()
    {
        if (settings::output_file_path.empty())
        {
            logger.error("No output file specified");
            exit(1);
        }
    }

    //
    // ================ ffmpeg_path ================
    //

    ffmpeg_path::ffmpeg_path()
    {
        name = "ffmpeg_path";
        description = "The path to the FFmpeg executable";
        shortcall = "-f";
        longcall = "--ffmpeg-path";
        required = true;
    }

    ffmpeg_path::~ffmpeg_path()
    {
    }

    void ffmpeg_path::execute(std::string arg)
    {
        logger.debug("Executing ffmpeg_path option with argument: " + arg);

        std::string value = get_arg_value(arg);

        if (value.empty())
        {
            logger.error("No value specified for ffmpeg_path option");
            exit(1);
        }

        settings::ffmpeg_path = value;
        this->executed = true;
    }

    void ffmpeg_path::check()
    {
        if (settings::ffmpeg_path.empty())
        {
            logger.error("No FFmpeg path specified");
            exit(1);
        }

        if (settings::skip_ffmpeg_check)
        {
            logger.warning("Skipping FFmpeg check");
            return;
        }

        std::string cmd = settings::ffmpeg_path;
        cmd += " -version > /dev/null 2>&1";
        if (system(cmd.c_str()) != 0)
        {
            logger.error("FFmpeg not found. Exiting...");
            return;
        }
        else
        {
            logger.info("FFmpeg found");
        }
    }

    //
    // ================ debug ================
    //

    // there is kind of fun bug (not really a bug) where no debug logs are being printed before execution of this option
    // try putting -d at the start of the arguments if you want to see output of handle_arguments function in main.cpp

    debug::debug()
    {
        name = "debug";
        description = "Enable debug mode";
        longcall = "--debug";
        required = false;
    }

    debug::~debug()
    {
    }

    void debug::execute(std::string arg)
    {
        logger.debug("Executing debug option with argument: " + arg);

        settings::debug = true;
        this->executed = true;
    }

    void debug::check()
    {
    }

    //
    // ================ decode ================
    //

    decode::decode()
    {
        name = "decode";
        description = "Decode a video to a file";
        longcall = "--decode";
        shortcall = "-d";
        required = false;
    }

    decode::~decode()
    {
    }

    void decode::execute(std::string arg)
    {
        logger.debug("Executing decode option with argument: " + arg);

        settings::decode = true;
        this->executed = true;
    }

    void decode::check()
    {
    }

    //
    // ================ width ================
    //

    width::width()
    {
        name = "width";
        description = "The width of the video";
        longcall = "--width";
        shortcall = "-w";
        required = false;
    }

    width::~width()
    {
    }

    void width::execute(std::string arg)
    {
        logger.debug("Executing width option with argument: " + arg);

        std::string value = get_arg_value(arg);

        if (value.empty())
        {
            logger.error("No value specified for width option");
            exit(1);
        }

        settings::video::width = std::stoi(value);
        this->executed = true;
    }

    void width::check()
    {
    }

    //
    // ================ height ================
    //

    height::height()
    {
        name = "height";
        description = "The height of the video";
        longcall = "--height";
        shortcall = "-h";
        required = false;
    }

    height::~height()
    {
    }

    void height::execute(std::string arg)
    {
        logger.debug("Executing height option with argument: " + arg);

        std::string value = get_arg_value(arg);

        if (value.empty())
        {
            logger.error("No value specified for height option");
            exit(1);
        }

        settings::video::height = std::stoi(value);
        this->executed = true;
    }

    void height::check()
    {
    }

    //
    // ================ fps ================
    //

    fps::fps()
    {
        name = "fps";
        description = "The frames per second of the video";
        longcall = "--fps";
        shortcall = "-f";
        required = false;
    }

    fps::~fps()
    {
    }

    void fps::execute(std::string arg)
    {
        logger.debug("Executing fps option with argument: " + arg);

        std::string value = get_arg_value(arg);

        if (value.empty())
        {
            logger.error("No value specified for fps option");
            exit(1);
        }

        settings::video::fps = std::stoi(value);
        this->executed = true;
    }

    void fps::check()
    {
    }

    //
    // ================ pixel_size ================
    //

    pixel_size::pixel_size()
    {
        name = "pixel_size";
        description = "The size of a pixel in the video";
        longcall = "--pixel-size";
        shortcall = "-p";
        required = false;
    }

    pixel_size::~pixel_size()
    {
    }

    void pixel_size::execute(std::string arg)
    {
        logger.debug("Executing pixel_size option with argument: " + arg);

        std::string value = get_arg_value(arg);

        if (value.empty())
        {
            logger.error("No value specified for pixel_size option");
            exit(1);
        }

        settings::video::pixel_size = std::stoi(value);
        this->executed = true;
    }

    void pixel_size::check()
    {
    }

    //
    // ================ use_color ================
    //

    use_color::use_color()
    {
        name = "use_color";
        description = "Use color in the video";
        longcall = "--use-color";
        shortcall = "-c";
        required = false;
    }

    use_color::~use_color()
    {
    }

    void use_color::execute(std::string arg)
    {
        logger.debug("Executing use_color option with argument: " + arg);

        settings::video::use_color = true;
        this->executed = true;
    }

    void use_color::check()
    {
    }

    //
    // ================ Global Argument List ================
    //

    std::vector<option *> defined_options = {
        new input_file(),
        new output_file(),
        new ffmpeg_path(),
        new debug(),
        new decode(),
        new width(),
        new height(),
        new fps(),
        new pixel_size(),
        new use_color()};
}