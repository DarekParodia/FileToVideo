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
#include "generator/fileio.h"
#include "generator/hash.h"

namespace generator
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
        generator::FileInput *input_file;
        void exec_ffmpeg();
        void generate_header();
        uint8_t *generate_frame(size_t frame_index);
        uint8_t *header;
        size_t header_size;
        size_t bits_per_frame;
        size_t total_frames;
        size_t video_duration;
    };
}

// Global generator object
extern generator::Generator gen;