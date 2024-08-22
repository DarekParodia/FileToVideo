#pragma once

#include <cstdint>

#include "io/video/video_input.h"

namespace io::video
{
    class YoutubeInput : public VideoInput
    {
    public:
        YoutubeInput();
        ~YoutubeInput();
        void open() override;
        void close() override;
        void update() override;
        uint8_t *readFrame() override;

    private:
        // not implemented
    };
}