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
        if (!this->input_file->isOpen())
            this->input_file->open();

        // Process frames
        this->output_file->clear();
        bool done = false;
        int frameCount = 1; // index 1 because first frame was read in calculate_requiraments
        uint8_t *buffer = nullptr;

        std::thread update_thread([&]()
                                  {
            while (!done && (frameCount < total_frames || total_frames == 0))
            {
                while (this->input_file->getBufferLenght() > settings::max_buffered_frames && !done && (frameCount < total_frames || total_frames == 0))
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }

                this->input_file->update();
                logger.debug("Buffered frame count: " + std::to_string(this->input_file->getBufferLenght()));
            } });

        while (frameCount < total_frames && !done)
        {
            while (this->input_file->getBufferLenght() == 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            buffer = this->input_file->readFrame();
            size_t dataSize = 0;
            uint8_t *decodedFrame = nullptr;
            if (frameCount < HEADER_FRAMES)
            {
                decodedFrame = this->decode_frame(buffer, &dataSize, true); // we don't need this anymore because header is being read in calculate_requiraments
            }
            else
            {
                decodedFrame = this->decode_frame(buffer, &dataSize, false);
            }

            free(buffer);

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
            settings::video::width = 0;
            settings::video::height = 0;
            settings::video::fps = 0;
            settings::video::pixel_size = 0;
            settings::video::color_space = 0;
            this->total_frames = 0;

            // 6 bytes for version
            memcpy(&this->version_major, current_byte, sizeof(uint16_t));
            current_byte += sizeof(uint16_t);
            memcpy(&this->version_minor, current_byte, sizeof(uint16_t));
            current_byte += sizeof(uint16_t);
            memcpy(&this->version_patch, current_byte, sizeof(uint16_t));
            current_byte += sizeof(uint16_t);

            // 12 bytes for video info
            memcpy(&settings::video::width, current_byte, sizeof(unsigned int));
            current_byte += sizeof(unsigned int);

            memcpy(&settings::video::height, current_byte, sizeof(unsigned int));
            current_byte += sizeof(unsigned int);

            memcpy(&settings::video::fps, current_byte, sizeof(unsigned int));
            current_byte += sizeof(unsigned int);

            // 16 bytes for pixel size, color space and total frames

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
            logger.info("Width: " + std::to_string(settings::video::width));
            logger.info("Height: " + std::to_string(settings::video::height));
            logger.info("FPS: " + std::to_string(settings::video::fps));
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

            // compare hashes
            *data_size = bits_in_frame;

            *data_size /= 8;
            *data_size -= 24;

            __uint128_t calculated_hash = io::hash(current_byte, *data_size);
            if (calculated_hash != hash)
            {
                logger.warning("Hashes do not match!");
                logger.warning("Frame index: " + std::to_string(frame_index));
                logger.warning("Calculated hash: " + bytes_to_hex_string((uint8_t *)&calculated_hash, 16));
                logger.warning("Expected hash: " + bytes_to_hex_string((uint8_t *)&hash, 16));
            }

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

    void Decoder::setInputFile(io::video::VideoInput *input_file)
    {
        this->input_file = input_file;
    }

    void Decoder::calculate_requiraments()
    {
        if (this->input_file == nullptr)
        {
            logger.error("Input file not set");
            exit(1);
        }

        // open file
        if (!this->input_file->isOpen())
            this->input_file->open();

        logger.info("===============");
        logger.info("Video info:");

        // read first frame (header) and retrieve video info
        uint8_t *header = this->input_file->readFrame();
        this->decode_frame(header, nullptr, true);

        this->frame_size = settings::video::width * settings::video::height * 3;
        logger.info("===============");
    }
}

decoder::Decoder dec;
