#include "video_gen.h"

namespace io
{
    VideoGen::VideoGen()
    {
        width = settings::video::width;
        height = settings::video::height;
        fps = settings::video::fps;
    }

    VideoGen::~VideoGen()
    {
        this->close();
    }

    size_t VideoGen::getWidth()
    {
        return width;
    }

    size_t VideoGen::getHeight()
    {
        return height;
    }

    size_t VideoGen::getBufferLenght()
    {
        return 0;
    }

    bool VideoGen::isOpen()
    {
        return is_open;
    }

    void VideoGen::open()
    {
        is_open = true;
    }

    void VideoGen::close()
    {
        is_open = false;
    }

    void VideoGen::writeFrame(uint8_t *frame)
    {
    }

    void VideoGen::setInfo(size_t width, size_t height, size_t fps)
    {
        this->width = width;
        this->height = height;
        this->fps = fps;
    }
}