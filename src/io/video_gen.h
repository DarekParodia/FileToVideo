#pragma once

#include "settings.h"

#include <cstdint>

namespace io
{
    class VideoGen
    {
    public:
        VideoGen();
        ~VideoGen();
        size_t getWidth();
        size_t getHeight();
        size_t getBufferLenght();
        bool isOpen();
        void open();
        void close();
        void writeFrame(uint8_t *frame);
        void setInfo(size_t width, size_t height, size_t fps);

    private:
        size_t width;
        size_t height;
        size_t fps;

        bool is_open = false;
    };
}