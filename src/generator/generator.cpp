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

            for (size_t i = 0; i < this->total_frames + RELIABILITY_FRAMES; i++)
            {
                uint8_t *frame_buffer = this->generate_frame(i);
                write(fd[1], frame_buffer, settings::video::width * settings::video::height * 3);
                logger.debug("Frame " + std::to_string(i + 1) + " / " + std::to_string(this->total_frames) + " generated");
                free(frame_buffer);
            }

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
        this->input_file = new generator::FileInput(settings::input_file_path);

        // frame capacity
        free(this->generate_frame_header(0, 0)); // to generate frame header size

        this->bits_per_frame = (settings::video::width / settings::video::pixel_size) * (settings::video::height / settings::video::pixel_size);
        if (settings::video::use_color)
            this->bits_per_frame *= 3; // rgb

        this->bits_per_frame -= this->frame_header_size * 8; // remove space for frame header

        // total frames
        this->total_frames = (size_t)ceil((double)(this->input_file->size()) / (double)(this->bits_per_frame / 8));
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
        logger.info("Bits per video frame: " + std::to_string(this->bits_per_frame));
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
        logger.debug("Header: " + bytes_to_bit_string(this->header, this->header_size));
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
        size_t frame_size = settings::video::width * settings::video::height * 3;
        size_t bytes_per_frame = ((settings::video::width / settings::video::pixel_size) * (settings::video::height / settings::video::pixel_size));
        bool isHeader = frame_index < HEADER_FRAMES;

        if (isHeader)
        {
            bytes_per_frame = ((settings::video::width / HEADER_PIXEL_SIZE) * (settings::video::height / HEADER_PIXEL_SIZE));
        }

        if (settings::video::use_color)
            bytes_per_frame *= 3;

        uint8_t *frame_data = static_cast<uint8_t *>(malloc(bytes_per_frame));
        uint8_t *frame = static_cast<uint8_t *>(malloc(frame_size));

        size_t pixel_counter = 0;

        memset(frame_data, 0, bytes_per_frame);
        memset(frame, 0xff / 2, frame_size); // after end of header fill the rest of the frame with gray color

        bytes_per_frame /= 8; // convert bytes to bits (artificial bits)

        if (isHeader)
        {
            memcpy(frame_data, this->header, this->header_size);
        }
        else
        {
            // generate frame header
            uint8_t *file_buffer = this->input_file->read(bytes_per_frame - this->frame_header_size);
            if (this->input_file->eof())
            {
                logger.warning("End of file reached");
                memset(file_buffer, 0, bytes_per_frame - this->frame_header_size);
            }
            __uint128_t hash = generator::hash(file_buffer, bytes_per_frame - this->frame_header_size);
            uint8_t *frame_header = this->generate_frame_header(frame_index, hash);

            // copy frame header to frame data
            memcpy(frame_data, frame_header, this->frame_header_size);

            // copy file buffer to frame data
            memcpy(frame_data + this->frame_header_size, file_buffer, bytes_per_frame - this->frame_header_size);

            free(file_buffer);
            free(frame_header);
        }

        // go through all pixels and set them
        size_t bit_counter = 0;
        uint8_t *current_byte = frame_data;
        if (!isHeader)
        {
            for (size_t i = 0; i < ((settings::video::width / settings::video::pixel_size) * (settings::video::height / settings::video::pixel_size)); i++)
            {
                if (settings::video::use_color)
                {
                    // get 3 bits from frame data
                    bool red = get_bit(*current_byte, bit_counter % 8);
                    bit_counter++;
                    if (bit_counter % 8 == 0)
                    {
                        current_byte++;
                    }
                    bool green = get_bit(*current_byte, bit_counter % 8);
                    bit_counter++;
                    if (bit_counter % 8 == 0)
                    {
                        current_byte++;
                    }
                    bool blue = get_bit(*current_byte, bit_counter % 8);
                    bit_counter++;
                    if (bit_counter % 8 == 0)
                    {
                        current_byte++;
                    }

                    utils::pixel p = {red ? 0xff : 0x00, green ? 0xff : 0x00, blue ? 0xff : 0x00};
                    this->set_byte(frame, i, p, settings::video::pixel_size);
                }
                else
                {
                    // get 1 bit from frame data
                    bool bit = get_bit(*current_byte, bit_counter % 8);
                    utils::pixel p = {bit ? 0xff : 0x00, bit ? 0xff : 0x00, bit ? 0xff : 0x00};
                    this->set_byte(frame, i, p, settings::video::pixel_size);
                    bit_counter++;
                    if (bit_counter % 8 == 0)
                    {
                        current_byte++;
                    }
                }
            }
        }
        else
        {
            for (size_t i = 0; i < ((settings::video::width / HEADER_PIXEL_SIZE) * (settings::video::height / HEADER_PIXEL_SIZE)); i++)
            {
                bool bit = get_bit(*current_byte, bit_counter % 8);
                utils::pixel p = {bit ? 0xff : 0x00, bit ? 0xff : 0x00, bit ? 0xff : 0x00};
                this->set_byte(frame, i, p, HEADER_PIXEL_SIZE);
                bit_counter++;
                if (bit_counter % 8 == 0)
                {
                    current_byte++;
                }
            }
        }
        return frame;
    }

    void Generator::set_byte(uint8_t *frame, size_t n, utils::pixel p, size_t pixel_size)
    {
        size_t row = n / (settings::video::width / pixel_size);
        uint8_t *current_byte = frame + (row * settings::video::width * 3 * pixel_size) + ((n % (settings::video::width / pixel_size)) * 3 * pixel_size);
        for (size_t i = 0; i < pixel_size; i++)
        {
            for (size_t j = 0; j < pixel_size; j++)
            {
                current_byte[0] = p.r;
                current_byte[1] = p.g;
                current_byte[2] = p.b;
                current_byte += 3;
            }
            current_byte += settings::video::width * 3 - (pixel_size * 3);
        }
    }
}
generator::Generator gen;
