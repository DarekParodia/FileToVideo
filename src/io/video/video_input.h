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
        size_t getFrameCount();
        bool hasFrame();
        virtual void open();
        virtual void close();
        virtual void update(); // updates queue with new frames (one frame per call)
        virtual uint8_t *readFrame(size_t &size);

    protected:
        size_t width;
        size_t height;
        size_t frame_size; // size of one frame in bytes. kind of unnecessary because it is always width * height * 3 (rgb) but whatever
        std::queue<uint8_t *> frame_buffer;
        const unsigned int max_frame_buffer_size = settings::max_buffered_frames;
    };
}