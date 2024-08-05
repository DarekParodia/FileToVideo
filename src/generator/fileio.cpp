#include "fileio.h"

namespace generator
{
    File::File(std::filesystem::path path)
    {
        this->path = path;
    }
    File::~File()
    {
        this->file.close();
    }

    void File::check_open()
    {
        if (!this->file.is_open())
        {
            try
            {
                this->file.open(this->path, std::ios::binary);
            }
            catch (std::exception &e)
            {
                logger.error("Failed to open file: " + this->path.string());
                logger.error(e.what());
                exit(1);
            }

            if (!this->file.is_open())
            {
                logger.error("Failed to open file: " + this->path.string());
                exit(1);
            }
        }
    }

    unsigned long File::size()
    {
        unsigned long size;
        try
        {
            size = std::filesystem::file_size(this->path);
        }
        catch (std::exception &e)
        {
            logger.error("Failed to get file size: " + this->path.string());
            logger.error(e.what());
            exit(1);
        }
        return size;
    }

}