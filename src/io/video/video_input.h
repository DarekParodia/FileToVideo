#pragma once

#include <queue>
#include <cstdint>

#include "settings.h"

namespace io::video
{
    class VideoInput
    {
    public:
        VideoInput();
        ~VideoInput();
        size_t getWidth();
        size_t getHeight();
        virtual void open();
        virtual void close();
        virtual uint8_t *readFrame(size_t &size);

    private:
        size_t width;
        size_t height;
        size_t frame_size; // size of one frame in bytes. kind of unnecessary because it is always width * height * 3 (rgb) but whatever
        std::queue<uint8_t *> frame_buffer;
        unsigned int frame_buffer_size;
    };
}