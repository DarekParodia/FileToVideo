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
        this->output_file = new generator::FileOutput(settings::output_file_path);

        AVFormatContext *pFormatCtx = nullptr;
        AVCodecContext *pCodecCtx = nullptr;
        AVFrame *pFrame = av_frame_alloc();
        AVPacket packet;
        int videoStreamIndex = -1;
        AVCodec *pCodec = nullptr;

        // Open video file
        if (avformat_open_input(&pFormatCtx, settings::input_file_path.c_str(), NULL, NULL) != 0)
        {
            logger.error("Failed to open file: " + settings::input_file_path);
            exit(1);
        }

        // Get stream info
        if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
        {
            logger.error("Failed to find stream info");
            exit(1);
        }

        // Find video stream
        for (int i = 0; i < pFormatCtx->nb_streams; i++)
        {
            if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                videoStreamIndex = i;
                break;
            }
        }

        if (videoStreamIndex == -1)
        {
            logger.error("Failed to find video stream");
            exit(1);
        }

        // Get codec parameters
        auto codecParameters = pFormatCtx->streams[videoStreamIndex]->codecpar;

        // Find decoder
        pCodec = const_cast<AVCodec *>(avcodec_find_decoder(this->codec_parameters->codec_id));
        if (pCodec == NULL)
        {
            logger.error("Failed to find decoder");
            exit(1);
        }

        // Allocate codec context
        pCodecCtx = avcodec_alloc_context3(pCodec);
        if (pCodecCtx == NULL)
        {
            logger.error("Failed to allocate codec context");
            exit(1);
        }

        // Copy codec parameters to codec context
        if (avcodec_parameters_to_context(pCodecCtx, codecParameters) < 0)
        {
            logger.error("Failed to copy codec parameters to codec context");
            exit(1);
        }

        // Open codec
        if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
        {
            logger.error("Failed to open codec");
            exit(1);
        }

        // Initialize variables
        int frameCount = 0;
        AVFrame *pFrameRGB = av_frame_alloc();
        if (pFrameRGB == NULL)
        {
            logger.error("Failed to allocate RGB frame");
            exit(1);
        }

        int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);
        uint8_t *buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
        av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);

        struct SwsContext *swsContext = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
        if (swsContext == NULL)
        {
            logger.error("Failed to create sws context");
            exit(1);
        }

        // Process frames
        while (av_read_frame(pFormatCtx, &packet) >= 0)
        {
            if (packet.stream_index == videoStreamIndex)
            {
                int response = avcodec_send_packet(pCodecCtx, &packet);
                if (response < 0)
                {
                    logger.error("Failed to send packet to codec");
                    av_packet_unref(&packet);
                    break;
                }

                response = avcodec_receive_frame(pCodecCtx, pFrame);
                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
                {
                    continue;
                }
                else if (response < 0)
                {
                    logger.error("Failed to receive frame from codec");
                    av_packet_unref(&packet);
                    break;
                }

                sws_scale(swsContext, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

                // Decode frame
                size_t dataSize = 0;
                uint8_t *decodedFrame;
                if (frameCount < HEADER_FRAMES)
                {
                    decodedFrame = this->decode_frame(buffer, &dataSize, true);
                }
                else
                {
                    decodedFrame = this->decode_frame(buffer, &dataSize, false);
                }

                // Write frame to output file if it is not header
                if (decodedFrame != nullptr)
                {
                    this->output_file->write(decodedFrame, dataSize);
                }

                frameCount++;

                av_packet_unref(&packet);
            }
        }

        // Cleanup
        av_frame_free(&pFrame);
        av_frame_free(&pFrameRGB);
        avcodec_free_context(&pCodecCtx);
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        av_freep(&buffer);
        sws_freeContext(swsContext);
    }

    uint8_t *Decoder::decode_frame(uint8_t *input_frame, size_t *data_size, bool isHeader)
    {
        // logger.debug("Decoding frame with size: " + std::to_string(this->frame_size) + " bytes");
        uint8_t *current_byte = input_frame;
        int pixel_counter = 0;
        if (isHeader)
        {
            // get specific pixels of header frames to read information about video encoding
            uint16_t version_major = 0;
            uint16_t version_minor = 0;
            uint16_t version_patch = 0;

            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 16; j++)
                {
                    utils::pixel p = this->get_pixel(input_frame, pixel_counter, HEADER_PIXEL_SIZE);
                    pixel_counter++;
                    uint8_t distance = utils::get_pixel_distance(p, utils::pixel(0, 0, 0));
                    if (distance > 128)
                    {
                        switch (i)
                        {
                        case 0:
                            // set bit of j-th position to 1
                            version_major |= 1 << j;
                            break;
                        case 1:
                            version_minor |= 1 << j;
                            break;
                        case 2:
                            version_patch |= 1 << j;
                            break;
                        }
                    }
                    else // it doesn't have to set to 0 because it is already 0 but who cares
                    {
                        switch (i)
                        {
                        case 0:
                            // set bit of j-th position to 0
                            version_major &= ~(1 << j);
                            break;
                        case 1:
                            version_minor &= ~(1 << j);
                            break;
                        case 2:
                            version_patch &= ~(1 << j);
                            break;
                        }
                    }
                }
            }

            logger.info("Detected Version: " + std::to_string(version_major) + "." + std::to_string(version_minor) + "." + std::to_string(version_patch));

            if (version_major != VERSION_MAJOR || version_minor != VERSION_MINOR || version_patch != VERSION_PATCH)
            {
                logger.warning("Unsupported encoding version. Use correct version of this program to decode or rerun the program with the correct argument to ignore."); // I might do multiversion support in the future but for now it is not needed
                exit(1);
            }

            // read video settings
            settings::video::pixel_size = 0;
            settings::video::color_space = 0;
            settings::video::use_color = false;

            pixel_counter += 4 * 8 * 3;  // skip width, height and fps pixels
            for (int i = 0; i < 32; i++) // pixel size
            {
                utils::pixel p = this->get_pixel(input_frame, pixel_counter, HEADER_PIXEL_SIZE);
                pixel_counter++;
                uint8_t distance = utils::get_pixel_distance(p, utils::pixel(0, 0, 0));
                if (distance > 128)
                {
                    settings::video::pixel_size |= 1 << i - 1; // minus 1 because it is unsigned int (no sign bit)
                }
                else
                {
                    settings::video::pixel_size &= ~(1 << i - 1);
                }
            }

            for (int i = 0; i < 32; i++) // color space
            {
                utils::pixel p = this->get_pixel(input_frame, pixel_counter, HEADER_PIXEL_SIZE);
                pixel_counter++;
                uint8_t distance = utils::get_pixel_distance(p, utils::pixel(0, 0, 0));
                if (distance > 128)
                {
                    settings::video::color_space |= 1 << i - 1;
                }
                else
                {
                    settings::video::color_space &= ~(1 << i - 1);
                }
            }

            // booleans
            for (int i = 0; i < 8; i++)
            {
                utils::pixel p = this->get_pixel(input_frame, pixel_counter, HEADER_PIXEL_SIZE);
                pixel_counter++;
                uint8_t distance = utils::get_pixel_distance(p, utils::pixel(0, 0, 0));
                if (distance > 128)
                {
                    switch (i)
                    {
                    case 7:
                        settings::video::use_color = true;
                        break;
                    }
                }
            }

            logger.debug("Detected video settings:");
            logger.debug("Width: " + std::to_string(settings::video::width));
            logger.debug("Height: " + std::to_string(settings::video::height));
            logger.debug("FPS: " + std::to_string(settings::video::fps));
            logger.debug("Pixel size: " + std::to_string(settings::video::pixel_size));
            logger.debug("Color space: " + std::to_string(settings::video::color_space));
            logger.debug("Total frames: " + std::to_string(this->total_frames));
            logger.debug("Use color: " + std::to_string(settings::video::use_color));

            return nullptr;
        }
        else
        {
            // decode frame
            // calculate storage size
            free(gen.generate_frame_header(0, 0)); // generate header to get header size
            size_t header_size = gen.frame_header_size;
            size_t bits_per_frame = (settings::video::width / settings::video::pixel_size) * (settings::video::height / settings::video::pixel_size);
            if (settings::video::use_color)
                bits_per_frame *= 3;                                                  // rgb
            size_t total_frame_bytes = bits_per_frame * 8;                            // total bits in frame
            bits_per_frame -= header_size * 8 * (settings::video::use_color ? 3 : 1); // remove header bits

            uint8_t *data = (uint8_t *)malloc(bits_per_frame / 8); // file data buffor
            memset(data, 0, bits_per_frame / 8);

            uint8_t *decoded_frame = (uint8_t *)malloc(total_frame_bytes); // decoded frame buffor

            // read header
            size_t frame_index = 0;
            __uint128_t hash = 0;

            size_t bit_counter = 0;

            // decode frame
            for (; bit_counter < bits_per_frame;)
            {
                utils::pixel p = this->get_pixel(input_frame, pixel_counter, settings::video::pixel_size);
                pixel_counter++;

                if (!settings::video::use_color)
                { // black and white (1 bit per pixel)
                    uint8_t distance = utils::get_pixel_distance(p, utils::pixel(0, 0, 0));
                    if (distance > 128)
                    {
                        decoded_frame[bit_counter / 8] |= 1 << (bit_counter % 8);
                    }
                    else
                    {
                        decoded_frame[bit_counter / 8] &= ~(1 << (bit_counter % 8));
                    }
                    bit_counter++;
                }
                else
                { // color (3 bits per pixel)
                    p = utils::get_pixel_distances(p, utils::pixel(0, 0, 0));

                    if (p.r > 128)
                    {
                        decoded_frame[bit_counter / 8] |= 1 << (bit_counter % 8);
                    }
                    else
                    {
                        decoded_frame[bit_counter / 8] &= ~(1 << (bit_counter % 8));
                    }
                    bit_counter++;

                    if (p.g > 128)
                    {
                        decoded_frame[bit_counter / 8] |= 1 << (bit_counter % 8);
                    }
                    else
                    {
                        decoded_frame[bit_counter / 8] &= ~(1 << (bit_counter % 8));
                    }
                    bit_counter++;

                    if (p.b > 128)
                    {
                        decoded_frame[bit_counter / 8] |= 1 << (bit_counter % 8);
                    }
                    else
                    {
                        decoded_frame[bit_counter / 8] &= ~(1 << (bit_counter % 8));
                    }
                    bit_counter++;
                }
            }

            // copy from decoded frame to variables
            memcpy(&frame_index, decoded_frame, sizeof(size_t));
            memcpy(&hash, decoded_frame + sizeof(size_t), sizeof(__uint128_t));

            logger.debug("Frame index decoded: " + std::to_string(frame_index));
            logger.debug("Hash decoded: " + bytes_to_hex_string((uint8_t *)&hash, 16));
            __uint128_t calculated_hash = generator::hash(decoded_frame + sizeof(size_t) + sizeof(__uint128_t), bits_per_frame / 8);

            bit_counter = 0;
        }
        return nullptr;
    }

    utils::pixel Decoder::get_pixel(uint8_t *data, size_t n, size_t pixel_size)
    {
        size_t total_pixels = pixel_size * pixel_size;
        utils::pixel *temp_pixels = (utils::pixel *)malloc(total_pixels * sizeof(utils::pixel));
        int pixel_counter = 0;
        uint8_t *current_byte = data + (n * 3 * pixel_size);
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
