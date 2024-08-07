#include "generator.h"

namespace generator
{
    Generator::Generator()
    {
        this->generate_header(); // to ensure that at least default header exists
    }

    Generator::~Generator()
    {
        delete this->input_file;
        free(this->header);
    }

    void Generator::generate()
    {
        logger.info("Generating video...");

        // initialize ffmpeg process
        int fd[2];
        if (pipe(fd) < 0)
        {
            logger.error("Failed to create pipe");
            exit(1);
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            logger.error("Failed to fork process");
            exit(1);
        }

        if (pid == 0)
        {
            // child process
            close(fd[1]); // close write end of the pipe

            // redirect stdin to read end of the pipe
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);

            // execute ffmpeg
            this->exec_ffmpeg();

            // if ffmpeg fails
            logger.error("Failed to execute ffmpeg");
            exit(1);
        }
        else
        {
            // parent process
            close(fd[0]); // close read end of the pipe

            // wait for ffmpeg to start (this has to change in the future)
            usleep(1000000);
            uint8_t *frame_buffer = this->generate_frame(0);

            logger.debug("Writing frame 0 to pipe");
            write(fd[1], frame_buffer, settings::video::width * settings::video::height * 3);
            logger.debug("Frame 0 written to pipe");

            // write(fd[1], frame_buffer, settings::video::width * settings::video::height * 3);

            close(fd[1]); // close write end of the pipe

            // wait for child process to finish
            waitpid(pid, NULL, 0);
        }
    }

    void Generator::calculate_requiraments()
    {
        // =====================
        //  actual calculations
        // =====================
        this->input_file = new generator::File(settings::input_file_path);

        // frame capacity
        free(this->generate_frame_header(0, 0)); // to generate frame header size

        this->bytes_per_frame = (settings::video::width / settings::video::pixel_size) * (settings::video::height / settings::video::pixel_size);
        this->bytes_per_frame *= settings::video::color_space;

        if (settings::video::use_color)
            this->bytes_per_frame *= 3; // rgb

        // total frames
        this->total_frames = (size_t)ceil((double)this->input_file->size() / (double)this->bytes_per_frame);
        this->total_frames += HEADER_FRAMES; // first 2 frames are reserved for header; more info in settings.h

        // video duration
        this->video_duration = this->total_frames / (((double)settings::video::fps) / 1000.0d); // total_frames / frames per milisecond = duration in miliseconds

        this->generate_header();

        // =====================
        //       log dump
        // =====================
        logger.info("Video Info:");
        logger.info("========================"); // just for seperation

        logger.info("Input file size: " + format_bytes(this->input_file->size()));
        logger.info("Video width: " + std::to_string(settings::video::width));
        logger.info("Video height: " + std::to_string(settings::video::height));
        logger.info("Video fps: " + std::to_string(settings::video::fps));
        logger.info("Video pixel size: " + std::to_string(settings::video::pixel_size));
        logger.info("Video color space: " + std::to_string(settings::video::color_space));
        logger.info("Use color: " + std::string(settings::video::use_color ? "true" : "false"));

        logger.info("========================");
        logger.info("Bytes per video frame: " + std::to_string(this->bytes_per_frame));
        logger.info("Total frames: " + std::to_string(this->total_frames));

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3) << this->video_duration / 1000.0 << " seconds";
        logger.info("Video Duration: " + oss.str());
        logger.info("========================");
    }
    void Generator::exec_ffmpeg()
    {
        std::string size = std::to_string(settings::video::width) + "x" + std::to_string(settings::video::height);

        // Allocate memory for each argument
        const char *args[] = {
            settings::ffmpeg_path.c_str(),
            "-y",
            "-f",
            "rawvideo",
            "-vcodec",
            "rawvideo",
            "-s",
            size.c_str(),
            "-pix_fmt",
            "rgb24",
            "-i",
            "pipe:0",
            "-an",
            settings::output_file_path.c_str(),
            NULL};

        logger.debug("Executing ffmpeg with args: " + char_array_to_string(args, 14));
        execlp(args[0], args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7],
               args[8], args[9], args[10], args[11], args[12], args[13], args[14], args[15]);
    }
    void Generator::generate_header()
    {
        // header can be seen only on the first frame of the video
        // it contains information about the video that can be used to decode it correctly

        // header format:
        // -- note: first 6 bytes are always for version to make sure that the decoder knows wich version of header it is reading because the header size may change in the future
        // 2 bytes - major version
        // 2 bytes - minor version
        // 2 bytes - patch version

        // -- note: i am keeping 4 bytes for width, height etc. because i am not sure if i will need more space in the future (probably not but it is better to be safe than sorry)
        // 4 bytes - width
        // 4 bytes - height
        // 4 bytes - fps
        // 4 bytes - pixel size
        // 4 bytes - color space
        // 8 bytes - total frames

        // -- note: 1 byte for booleans to save space (may be changed in the future if needed)
        // byte 1
        //  bit 1 - use color

        // header will always be encoded in black and white and have pixel size of 2

        uint16_t ver_major = VERSION_MAJOR;
        uint16_t ver_minor = VERSION_MINOR;
        uint16_t ver_patch = VERSION_PATCH;

        free(this->header);
        this->header_size = 6 + 28 + 1; // 6 bytes for version, 20 bytes for video settings, 1 byte for booleans
        this->header = static_cast<uint8_t *>(malloc(this->header_size));

        // write to header buffer
        memcpy(this->header, &ver_major, 2);
        memcpy(this->header + 2, &ver_minor, 2);
        memcpy(this->header + 4, &ver_patch, 2);

        memcpy(this->header + 6, &settings::video::width, 4);
        memcpy(this->header + 10, &settings::video::height, 4);
        memcpy(this->header + 14, &settings::video::fps, 4);
        memcpy(this->header + 18, &settings::video::pixel_size, 4);
        memcpy(this->header + 22, &settings::video::color_space, 4);
        memcpy(this->header + 26, &this->total_frames, 8);

        uint8_t booleans = 0;
        booleans |= ((uint8_t)settings::video::use_color << 0); // use color

        memcpy(this->header + 32, &booleans, 1);
        logger.debug("Header size: " + std::to_string(this->header_size));
        logger.debug("Header: " + bytes_to_hex_string(this->header, this->header_size));
    }
    uint8_t *Generator::generate_frame_header(size_t frame_index, __uint128_t hash)
    {
        // frame header format:
        // 8 bytes - frame index
        // 16 bytes (128 bits) - hash

        this->frame_header_size = 8 + 16;
        uint8_t *frame_header = static_cast<uint8_t *>(malloc(this->frame_header_size));

        memcpy(frame_header, &frame_index, 8);
        memcpy(frame_header + 8, &hash, 16);

        return frame_header;
    }
    uint8_t *Generator::generate_frame(size_t frame_index)
    {
        // create data buffer
        uint8_t *frame = static_cast<uint8_t *>(malloc(settings::video::width * settings::video::height * 3));
        uint8_t *current_pixel = frame;

        memset(frame, 0, settings::video::width * settings::video::height * 3);

        // generate frame header (hardcoded for now)
        if (frame_index < HEADER_FRAMES)
        {
            for (size_t i = 0; i < this->header_size; i++)
            {
                for (size_t j = 0; j < HEADER_PIXEL_SIZE; j++)
                {
                    for (size_t k = 0; k < HEADER_PIXEL_SIZE; k++)
                    {
                        size_t height_off = settings::video::width * 3 * k;
                        current_pixel[0 + height_off] = this->header[i];
                        current_pixel[1 + height_off] = this->header[i];
                        current_pixel[2 + height_off] = this->header[i];
                    }
                    current_pixel += 3;
                }
            }
        }
        return frame;
    }
}
generator::Generator gen;
