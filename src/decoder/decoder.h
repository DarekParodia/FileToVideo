#pragma once

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#include "settings.h"
#include "utils/logger.h"
#include "utils/general.h"
#include "utils/pixel.h"
#include "generator/fileio.h"

namespace decoder
{
    class Decoder
    {
    public:
        Decoder();
        ~Decoder();
        void decode();
        void calculate_requiraments();

    private:
        AVFormatContext *format_context;
        AVCodecParameters *codec_parameters;
        int video_stream_index;
        uint8_t *get_video_frame(size_t frame_index);
        uint8_t *decode_frame(size_t frame_index, size_t *data_size);
        size_t frame_size;
        size_t total_frames;
        generator::FileOutput *output_file;
    };
}

// Global decoder object
extern decoder::Decoder dec;