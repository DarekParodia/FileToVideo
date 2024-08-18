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
        logger.debug("Decoding frame with size: " + std::to_string(this->frame_size) + " bytes");
        uint8_t *current_byte = input_frame;

        // decode frame
        // calculate storage size
        free(gen.generate_frame_header(0, 0)); // generate header to get header size
        size_t header_size = gen.frame_header_size;
        size_t bits_in_frame = settings::video::width * settings::video::height;

        bool use_color = settings::video::use_color && !isHeader;

        if (use_color)
        {
            bits_in_frame *= 3;
        }

        uint8_t *decoded_data = (uint8_t *)malloc((bits_in_frame / 8) * sizeof(uint8_t));
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
                decoded_data[current_bit / 8] |= (pixel.r > 127) << 7 - (current_bit % 8);
                current_bit++;
                decoded_data[current_bit / 8] |= (pixel.g > 127) << 7 - (current_bit % 8);
                current_bit++;
                decoded_data[current_bit / 8] |= (pixel.b > 127) << 7 - (current_bit % 8);
                current_bit++;
            }
            else
            {
                uint8_t distance = utils::get_pixel_distance(pixel, utils::pixel(0, 0, 0));
                decoded_data[current_bit / 8] |= (distance > 127) << 7 - (current_bit % 8);
                current_bit++;
            }
        }
        if (isHeader)
        {
            logger.debug("Header data: " + bytes_to_bit_string(decoded_data, 35));
        }

        // now all bits from video frame are stored in decoded_data
        // we need to just copy it
        current_byte = decoded_data;

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

            // 1 byte for booleans

            // bit 0 - use_color
            settings::video::use_color = *current_byte & 1;

            logger.info("Version: " + std::to_string(this->version_major) + "." + std::to_string(this->version_minor) + "." + std::to_string(this->version_patch));
            logger.info("Pixel size: " + std::to_string(settings::video::pixel_size));
            logger.info("Color space: " + std::to_string(settings::video::color_space));
            logger.info("Total frames: " + std::to_string(this->total_frames));
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
            logger.debug("Data: " + bytes_to_hex_string(current_byte, 32));

            return current_byte;
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
