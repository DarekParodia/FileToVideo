#pragma once

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#include "settings.h"
#include "utils/logger.h"

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
    };
}

// Global decoder object
extern decoder::Decoder dec;