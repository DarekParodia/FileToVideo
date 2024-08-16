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

        // read frame data and write to file
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
        this->codec_parameters = pFormatCtx->streams[videoStreamIndex]->codecpar;

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
        if (avcodec_parameters_to_context(pCodecCtx, this->codec_parameters) < 0)
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

        // Read frames
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

        while (av_read_frame(pFormatCtx, &packet) >= 0)
        {
            if (packet.stream_index == videoStreamIndex)
            {
                int response = avcodec_send_packet(pCodecCtx, &packet);
                if (response < 0)
                {
                    logger.error("Failed to send packet to codec");
                    av_packet_unref(&packet); // Unreference the packet to avoid memory leak
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
                    av_packet_unref(&packet); // Unreference the packet to avoid memory leak
                    break;
                }

                sws_scale(swsContext, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

                // Decode frame
                size_t dataSize = 0;
                uint8_t *decodedFrame = this->decode_frame(buffer, &dataSize, frameCount == 0);
                if (decodedFrame != nullptr)
                {
                    if (frameCount < HEADER_FRAMES)
                    {
                        this->decode_frame(buffer, &dataSize, true);
                    }
                    else
                    {
                        this->decode_frame(buffer, &dataSize, false);
                    }
                }

                frameCount++;
            }
            av_packet_unref(&packet);
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
        if (isHeader)
        {
            // get specific pixels of header frames to read information about video encoding
            uint16_t version_major = 0;
            uint16_t version_minor = 0;
            uint16_t version_patch = 0;

            uint8_t *current_byte = input_frame;

            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 8; j++)
                {
                    utils::pixel p = this->get_pixel(input_frame, (i * 8) + j, HEADER_PIXEL_SIZE);
                    uint8_t distance = utils::get_pixel_distance(p, utils::pixel(0, 0, 0));
                    if (distance > 128)
                    {
                        switch (i)
                        {
                        case 0:
                            version_major |= 1 << (i * 8) + j;
                            break;
                        case 1:
                            version_minor |= 1 << (i * 8) + j;
                            break;
                        case 2:
                            version_patch |= 1 << (i * 8) + j;
                            break;
                        }
                    }
                    else
                    {
                        switch (i)
                        {
                        case 0:
                            version_major &= ~(1 << (i * 8) + j);
                            break;
                        case 1:
                            version_minor &= ~(1 << (i * 8) + j);
                            break;
                        case 2:
                            version_patch &= ~(1 << (i * 8) + j);
                            break;
                        }
                    }
                }
            }

            logger.debug("Version: " + std::to_string(version_major) + "." + std::to_string(version_minor) + "." + std::to_string(version_patch));

            return nullptr;
        }

        return nullptr;
    }

    utils::pixel Decoder::get_pixel(uint8_t *data, size_t n, size_t pixel_size)
    {
        utils::pixel *pixel_data = new utils::pixel[pixel_size * pixel_size];
        for (size_t i = 0; i < pixel_size; i++)
        {
            for (size_t j = 0; j < pixel_size * 3; j++)
            {
                uint8_t *data_index = data + j + (i * settings::video::width * 3);
                pixel_data[j + (i * pixel_size)] = utils::pixel(data_index[0], data_index[1], data_index[2]);
            }
        }
        utils::pixel average = utils::get_average_color(pixel_data, pixel_size * pixel_size);
        delete[] pixel_data;
        return average;
    }

    uint8_t *Decoder::get_video_frame(size_t frame_index)
    {
        logger.debug("Getting frame: " + std::to_string(frame_index));

        AVPacket packet;
        AVFrame *frame = av_frame_alloc();

        if (!frame)
        {
            logger.error("Failed to allocate frame");
            return nullptr;
        }

        this->format_context = avformat_alloc_context();
        if (!this->format_context)
        {
            logger.error("Failed to allocate format context");
            av_frame_free(&frame);
            return nullptr;
        }

        // Open file
        if (avformat_open_input(&this->format_context, settings::input_file_path.c_str(), NULL, NULL) != 0)
        {
            logger.error("Failed to open file: " + settings::input_file_path);
            av_frame_free(&frame);
            return nullptr;
        }

        // Get stream info
        if (avformat_find_stream_info(this->format_context, NULL) < 0)
        {
            logger.error("Failed to find stream info");
            av_frame_free(&frame);
            return nullptr;
        }

        // Find video stream
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
            av_frame_free(&frame);
            return nullptr;
        }

        // Read frame
        this->codec_parameters = this->format_context->streams[this->video_stream_index]->codecpar;
        AVCodec *codec = const_cast<AVCodec *>(avcodec_find_decoder(this->codec_parameters->codec_id));

        if (!codec)
        {
            logger.error("Failed to find codec");
            av_frame_free(&frame);
            return nullptr;
        }

        // Open codec
        AVCodecContext *codec_context = avcodec_alloc_context3(codec);
        if (!codec_context)
        {
            logger.error("Failed to allocate codec context");
            av_frame_free(&frame);
            return nullptr;
        }

        // Copy codec parameters
        if (avcodec_parameters_to_context(codec_context, this->codec_parameters) < 0)
        {
            logger.error("Failed to copy codec parameters to codec context");
            avcodec_free_context(&codec_context); // Free codec context before returning
            av_frame_free(&frame);
            return nullptr;
        }

        // Init codec
        if (avcodec_open2(codec_context, codec, NULL) < 0)
        {
            logger.error("Failed to open codec");
            avcodec_free_context(&codec_context); // Free codec context before returning
            av_frame_free(&frame);
            return nullptr;
        }

        // Read frames until we reach the desired frame
        int frame_count = 0;
        bool frame_found = false;
        while (av_read_frame(this->format_context, &packet) >= 0 && !frame_found)
        {
            if (packet.stream_index == this->video_stream_index)
            {
                int response = avcodec_send_packet(codec_context, &packet);
                if (response < 0)
                {
                    logger.error("Failed to send packet to codec");
                    av_packet_unref(&packet); // Unreference the packet to avoid memory leak
                    av_frame_free(&frame);
                    return nullptr;
                }

                response = avcodec_receive_frame(codec_context, frame);
                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
                {
                    continue;
                }
                else if (response < 0)
                {
                    logger.error("Failed to receive frame from codec");
                    av_packet_unref(&packet); // Unreference the packet to avoid memory leak
                    av_frame_free(&frame);
                    return nullptr;
                }

                if (frame_count == frame_index)
                {
                    frame_found = true;
                }
                frame_count++;
            }
            av_packet_unref(&packet);
        }

        // Convert frame to RGB
        AVFrame *rgb_frame = av_frame_alloc();
        if (!rgb_frame)
        {
            logger.error("Failed to allocate RGB frame");
            av_frame_free(&frame);
            return nullptr;
        }

        int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codec_context->width, codec_context->height, 1);
        uint8_t *buffer = (uint8_t *)av_malloc(num_bytes * sizeof(uint8_t));
        av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, buffer, AV_PIX_FMT_RGB24, codec_context->width, codec_context->height, 1);

        struct SwsContext *sws_context = sws_getContext(codec_context->width, codec_context->height, codec_context->pix_fmt, codec_context->width, codec_context->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
        if (!sws_context)
        {
            logger.error("Failed to create sws context");
            av_frame_free(&frame);
            av_frame_free(&rgb_frame);
            av_freep(&buffer);
            return nullptr;
        }

        sws_scale(sws_context, frame->data, frame->linesize, 0, codec_context->height, rgb_frame->data, rgb_frame->linesize);

        // Cleanup
        // av_packet_unref(&packet);
        // av_frame_free(&frame);
        // av_frame_free(&rgb_frame);
        // sws_freeContext(sws_context);

        return buffer;
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
