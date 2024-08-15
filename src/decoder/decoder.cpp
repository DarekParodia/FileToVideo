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

        // header frames
        for (int i = 0; i < HEADER_FRAMES; i++)
        {
            this->decode_frame(i, nullptr);
        }

        for (int i = HEADER_FRAMES; i < this->total_frames; i++)
        {
            size_t data_size;
            uint8_t *data = this->decode_frame(i, &data_size);
            this->output_file->write(data, data_size);
            delete[] data;
        }
    }
    uint8_t *Decoder::decode_frame(size_t frame_index, size_t *data_size)
    {
        uint8_t *input_frame = this->get_video_frame(frame_index);

        if (frame_index >= this->total_frames)
        {
            logger.error("Frame index out of range");
            exit(1);
        }

        if (frame_index < HEADER_FRAMES)
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
        size_t offset = n * pixel_size;
        return utils::pixel(data[offset], data[offset + 1], data[offset + 2]);
    }

    uint8_t *Decoder::get_video_frame(size_t frame_index)
    {
        AVPacket packet;
        av_init_packet(&packet);

        while (true)
        {
            if (av_read_frame(this->format_context, &packet) < 0)
            {
                logger.error("Failed to read frame");
                exit(1);
            }

            if (packet.stream_index == this->video_stream_index)
            {
                break;
            }
        }

        AVFrame *frame = av_frame_alloc();
        if (!frame)
        {
            logger.error("Failed to allocate frame");
            exit(1);
        }

        int response = avcodec_send_packet(this->format_context->streams[this->video_stream_index]->codecpar->codec_id, &packet);
        if (response < 0)
        {
            logger.error("Failed to send packet");
            exit(1);
        }

        response = avcodec_receive_frame(this->format_context->streams[this->video_stream_index]->codecpar->codec_id, frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
        {
            logger.error("Failed to receive frame");
            exit(1);
        }

        av_packet_unref(&packet);

        return frame->data[0];
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
