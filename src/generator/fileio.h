#pragma once

#include <string>
#include <fstream>
#include <filesystem>

#include "utils/logger.h"

namespace generator
{
    class File
    {
    public:
        File(std::filesystem::path path);
        ~File();
        unsigned long size();

    private:
        std::filesystem::path path;
        std::ifstream file;
        void check_open();
    };
}