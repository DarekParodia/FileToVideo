#pragma once

#include "video_input.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <cstdio>
#include <string>

#include "utils/logger.h"

namespace io::video
{
    class FileInput : public VideoInput
    {
    public:
        FileInput(std::string filename);
        ~FileInput();
        void open() override;
        void close() override;
        uint8_t *readFrame();

    private:
        std::string filename;
        AVFormatContext *pFormatCtx = nullptr;
        AVCodecContext *pCodecCtx = nullptr;
        AVFrame *pFrame = nullptr;
        AVFrame *pFrameRGB = nullptr;
        AVPacket packet;
        int videoStreamIndex = -1;
        int numBytes = 0;
        AVCodec *pCodec = nullptr;
        uint8_t *buffer = nullptr;
        struct SwsContext *sws_ctx = nullptr;
    };
}