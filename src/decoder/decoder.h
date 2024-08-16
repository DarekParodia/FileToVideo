#pragma once

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
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
        utils::pixel get_pixel(uint8_t *data, size_t n, size_t pixel_size);
        uint8_t *decode_frame(uint8_t *input_frame, size_t *data_size, bool isHeader);
        size_t frame_size;
        size_t total_frames;
        generator::FileOutput *output_file;
        uint16_t version_major;
        uint16_t version_minor;
        uint16_t version_patch;
    };
}

// Global decoder object
extern decoder::Decoder dec;