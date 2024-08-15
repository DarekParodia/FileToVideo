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
        if (frame_index >= this->total_frames)
        {
            logger.error("Frame index out of range");
            exit(1);
        }

        if (frame_index < HEADER_FRAMES)
        {
            // get specific pixels of header frames to read information about video encoding
            return nullptr;
        }

        return nullptr;
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
