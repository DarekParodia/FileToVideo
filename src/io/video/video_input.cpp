#include "video_input.h"

namespace io::video
{
    VideoInput::VideoInput()
    {
        width = settings::video::width;
        height = settings::video::height;
        frame_size = width * height * 3;
        frame_buffer_size = 0;
    }

    VideoInput::~VideoInput()
    {
        close();
    }

    size_t VideoInput::getWidth()
    {
        return width;
    }

    size_t VideoInput::getHeight()
    {
        return height;
    }

    void VideoInput::open()
    {
        // not implemented
    }

    void VideoInput::close()
    {
        // not implemented
    }

    uint8_t *VideoInput::readFrame(size_t &size)
    {
        // not implemented
    }
}