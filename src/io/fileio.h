#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <cstring>

#include "utils/logger.h"
#include "utils/general.h"

namespace io
{
    class FileInput
    {
    public:
        FileInput(std::filesystem::path path);
        ~FileInput();
        unsigned long size();
        long bytes_left(size_t start);
        uint8_t *read(size_t start, size_t size);
        uint8_t *read(size_t size);
        bool eof();

    private:
        std::filesystem::path path;
        std::ifstream file;
        size_t pointer;
        void check_open();
    };

    class FileOutput
    {
    public:
        FileOutput(std::filesystem::path path);
        ~FileOutput();
        void write(uint8_t *data, size_t size);
        void check_open();
        void clear();
        size_t get_written_bytes();

    private:
        std::filesystem::path path;
        std::ofstream file;
        size_t written_bytes;
    };
}