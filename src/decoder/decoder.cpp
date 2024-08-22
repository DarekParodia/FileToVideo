#include "decoder.h"

namespace decoder
{
    Decoder::Decoder()
    {
    }
    Decoder::~Decoder()
    {
    }

    void Decoder::decode()
    {
        this->output_file = new io::FileOutput(settings::output_file_path);
        this->input_file = new io::video::FileInput(settings::input_file_path);

        this->input_file->open();

        // Process frames
        this->output_file->clear();
        bool done = false;
        int frameCount = 0;
        uint8_t *buffer = nullptr;

        std::thread update_thread([&]()
                                  {
            while (!done && (frameCount < total_frames || total_frames == 0))
            {
                while (this->input_file->getFrameCount() > settings::max_buffered_frames && !done && (frameCount < total_frames || total_frames == 0))
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }

                this->input_file->update();
                logger.debug("Buffered frame count: " + std::to_string(this->input_file->getFrameCount()));
            } });

        while (frameCount < total_frames && !done)
        {
            while (this->input_file->getFrameCount() == 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            buffer = this->input_file->readFrame();
            size_t dataSize = 0;
            uint8_t *decodedFrame = nullptr;
            if (frameCount < HEADER_FRAMES)
            {
                decodedFrame = this->decode_frame(buffer, &dataSize, true);
            }
            else
            {
                decodedFrame = this->decode_frame(buffer, &dataSize, false);
            }

            // Write frame to output file if it is not header
            if (decodedFrame != nullptr && frameCount >= HEADER_FRAMES)
            {
                logger.debug("Writing frame to file");
                if (this->total_bytes - this->output_file->get_written_bytes() < dataSize)
                {
                    dataSize = this->total_bytes - this->output_file->get_written_bytes();
                    done = true;
                    logger.debug("Done decoding");
                }
                this->output_file->write(decodedFrame, dataSize);
            }

            frameCount++;

            if (frameCount >= this->total_frames)
            {
                break;
            }
        }
        update_thread.join();
    }
    uint8_t *Decoder::decode_frame(uint8_t *input_frame, size_t *data_size, bool isHeader)
    {
        logger.debug("Decoding frame with size: " + std::to_string(this->frame_size) + " bytes");
        uint8_t *current_byte = input_frame;

        // decode frame
        // calculate storage size
        free(gen.generate_frame_header(0, 0)); // generate header to get header size
        size_t header_size = gen.frame_header_size;
        size_t bits_in_frame = (settings::video::width / settings::video::pixel_size) * (settings::video::height / settings::video::pixel_size);

        bool use_color = settings::video::use_color && !isHeader;

        if (use_color)
        {
            bits_in_frame *= 3;
        }

        uint8_t *decoded_data = (uint8_t *)malloc((bits_in_frame / 8) * sizeof(uint8_t));
        uint8_t *decoded_data_start = decoded_data;
        memset(decoded_data, 0, (bits_in_frame / 8) * sizeof(uint8_t));

        // go trough all pixels
        size_t current_bit = 0;
        for (size_t i = 0; i < (settings::video::width / (isHeader ? HEADER_PIXEL_SIZE : settings::video::pixel_size)) * (settings::video::height / (isHeader ? HEADER_PIXEL_SIZE : settings::video::pixel_size)); i++)
        {
            // get pixel
            utils::pixel pixel = this->get_pixel(input_frame, i, isHeader ? HEADER_PIXEL_SIZE : settings::video::pixel_size);

            if (use_color)
            {
                pixel = utils::get_pixel_distances(pixel, utils::pixel(0, 0, 0));
                decoded_data[current_bit / 8] |= (pixel.r > 127) << (current_bit % 8);
                current_bit++;
                decoded_data[current_bit / 8] |= (pixel.g > 127) << (current_bit % 8);
                current_bit++;
                decoded_data[current_bit / 8] |= (pixel.b > 127) << (current_bit % 8);
                current_bit++;
            }
            else
            {
                uint8_t distance = utils::get_pixel_distance(pixel, utils::pixel(0, 0, 0));
                decoded_data[current_bit / 8] |= (distance > 127) << (current_bit % 8);
                current_bit++;
            }
        }
        if (isHeader)
        {
            logger.debug("Header data: " + bytes_to_bit_string(decoded_data, 35));
        }

        // now all bits from video frame are stored in decoded_data
        // we need to just copy it
        current_byte = decoded_data_start;

        if (isHeader)
        {
            // 6 bytes for version
            memcpy(&this->version_major, current_byte, sizeof(uint16_t));
            current_byte += sizeof(uint16_t);
            memcpy(&this->version_minor, current_byte, sizeof(uint16_t));
            current_byte += sizeof(uint16_t);
            memcpy(&this->version_patch, current_byte, sizeof(uint16_t));
            current_byte += sizeof(uint16_t);

            // skip 12 bytes of width, height and fps (we arleadly know that)
            current_byte += 12;

            // 16 bytes for pixel size, color space and total frames
            settings::video::pixel_size = 0;
            settings::video::color_space = 0;
            this->total_frames = 0;

            memcpy(&settings::video::pixel_size, current_byte, sizeof(unsigned int));
            current_byte += sizeof(unsigned int);

            memcpy(&settings::video::color_space, current_byte, sizeof(unsigned int));
            current_byte += sizeof(unsigned int);

            memcpy(&this->total_frames, current_byte, sizeof(size_t));
            current_byte += sizeof(size_t);

            memcpy(&this->total_bytes, current_byte, sizeof(size_t));
            current_byte += sizeof(size_t);

            // 1 byte for booleans

            // bit 0 - use_color
            settings::video::use_color = *current_byte & 1;

            logger.info("Version: " + std::to_string(this->version_major) + "." + std::to_string(this->version_minor) + "." + std::to_string(this->version_patch));
            logger.info("Pixel size: " + std::to_string(settings::video::pixel_size));
            logger.info("Color space: " + std::to_string(settings::video::color_space));
            logger.info("Total frames: " + std::to_string(this->total_frames));
            logger.info("Total bytes: " + std::to_string(this->total_bytes));
            logger.info("Use color: " + std::to_string(settings::video::use_color));
        }
        else
        {
            // first 24 bytes are header
            size_t frame_index = 0;
            __uint128_t hash = 0;

            // copy header
            memcpy(&frame_index, current_byte, sizeof(size_t));
            current_byte += sizeof(size_t);
            memcpy(&hash, current_byte, sizeof(__uint128_t));
            current_byte += sizeof(__uint128_t);

            logger.debug("Frame index: " + std::to_string(frame_index));
            logger.debug("Hash: " + bytes_to_hex_string((uint8_t *)&hash, 16));

            *data_size = bits_in_frame;

            *data_size /= 8;
            *data_size -= 24;

            return current_byte;
        }
        return current_byte;
    }

    utils::pixel Decoder::get_pixel(uint8_t *data, size_t n, size_t pixel_size)
    {
        size_t total_pixels = pixel_size * pixel_size;
        utils::pixel *temp_pixels = (utils::pixel *)malloc(total_pixels * sizeof(utils::pixel));
        int pixel_counter = 0;
        size_t row = n / (settings::video::width / pixel_size);
        uint8_t *current_byte = data + (row * settings::video::width * 3 * pixel_size) + ((n % (settings::video::width / pixel_size)) * 3 * pixel_size);
        // logger.debug("pixel n:" + std::to_string(n));
        for (size_t i = 0; i < pixel_size; ++i)
        {
            for (size_t j = 0; j < pixel_size; ++j)
            {
                temp_pixels[pixel_counter] = utils::pixel(current_byte[0], current_byte[1], current_byte[2]);
                // logger.debug("Pixel address: 0x" + std::to_string((uintptr_t)(current_byte - data)));
                // logger.debug("Pixel r: " + std::to_string(current_byte[0]) + " g: " + std::to_string(current_byte[1]) + " b: " + std::to_string(current_byte[2]));
                pixel_counter++;
                current_byte += 3;
            }
            current_byte += settings::video::width * 3 - (pixel_size * 3); // skip to next row of pixels
        }

        // Calculate the average color of all pixels in the square area.
        utils::pixel average = utils::get_average_color(temp_pixels, total_pixels);

        // Clean up the temporary pixel array.
        delete[] temp_pixels;

        return average;
    }

    void Decoder::calculate_requiraments()
    {
        // open file
        if (avformat_open_input(&this->format_context, settings::input_file_path.c_str(), NULL, NULL) != 0)
        {
            logger.error("Failed to open file: " + settings::input_file_path);
            exit(1);
        }

        // get stream info
        if (avformat_find_stream_info(this->format_context, NULL) < 0)
        {
            logger.error("Failed to find stream info");
            exit(1);
        }

        // find video stream
        this->video_stream_index = -1;
        for (int i = 0; i < this->format_context->nb_streams; i++)
        {
            if (this->format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                this->video_stream_index = i;
                break;
            }
        }

        if (this->video_stream_index == -1)
        {
            logger.error("Failed to find video stream");
            exit(1);
        }

        this->codec_parameters = this->format_context->streams[this->video_stream_index]->codecpar;

        // set settings
        settings::video::width = this->codec_parameters->width;
        settings::video::height = this->codec_parameters->height;
        settings::video::fps = av_q2d(this->format_context->streams[this->video_stream_index]->avg_frame_rate);

        this->frame_size = this->codec_parameters->width * this->codec_parameters->height * 3;
        this->total_frames = this->format_context->streams[this->video_stream_index]->nb_frames;

        // print video info
        logger.info("Video info:");
        logger.info("Width: " + std::to_string(settings::video::width));
        logger.info("Height: " + std::to_string(settings::video::height));
        logger.info("FPS: " + std::to_string(settings::video::fps));
        logger.info("Total frames: " + std::to_string(this->total_frames));
    }
}

decoder::Decoder dec;
