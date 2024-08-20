#pragma once

#include <cstring>
#include <cmath>
#include <iomanip>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "settings.h"
#include "utils/logger.h"
#include "utils/general.h"
#include "utils/pixel.h"
#include "io/fileio.h"
#include "io/hash.h"

namespace io
{
    class Generator
    {
    public:
        Generator();
        ~Generator();
        void generate();
        void calculate_requiraments();
        uint8_t *generate_frame_header(size_t frame_index, __uint128_t hash);
        size_t frame_header_size;

    private:
        io::FileInput *input_file;
        void exec_ffmpeg();
        void generate_header();
        void set_byte(uint8_t *frame, size_t n, utils::pixel p, size_t pixel_size);
        uint8_t *generate_frame(size_t frame_index);
        uint8_t *header;
        size_t header_size;
        size_t bits_per_frame;
        size_t total_frames;
        size_t video_duration;
        size_t input_file_size;
    };
}

// Global generator object
extern io::Generator gen;