#include "fileio.h"

namespace generator
{
    FileInput::FileInput(std::filesystem::path path)
    {
        this->path = path;
    }
    FileInput::~FileInput()
    {
        this->file.close();
    }

    uint8_t *FileInput::read(size_t start, size_t size)
    {
        this->check_open();

        uint8_t *data = new uint8_t[size];
        this->file.seekg(start);
        this->file.read((char *)data, size);

        return data;
    }

    uint8_t *FileInput::read(size_t size)
    {
        this->pointer += size;
        return this->read(this->pointer - size, size);
    }

    void FileInput::check_open()
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

    bool FileInput::eof()
    {
        return this->file.eof();
    }

    unsigned long FileInput::size()
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

    long FileInput::bytes_left(size_t start)
    {
        return this->size() - start;
    }

    //
    // ============================== FILE OUTPUT ==============================
    //

    FileOutput::FileOutput(std::filesystem::path path)
    {
        this->path = path;
    }
    FileOutput::~FileOutput()
    {
        this->file.flush();
        this->file.close();
    }

    void FileOutput::write(uint8_t *data, size_t size)
    {
        this->check_open();

        if (!this->file.is_open())
        {
            logger.error("Error: Output file is not open.");
            return;
        }

        if (data == nullptr || size == 0)
        {
            logger.warning("Warning: Attempted to write null or zero-sized data.");
            return;
        }

        this->file << std::string((char *)data, size);

        if (!this->file.good())
        {
            logger.error("File stream is in an error state.");
        }

        if (!this->file)
        {
            logger.error("Error writing to file: " + this->path.string());
        }
        else
        {
            logger.debug("Successfully wrote " + std::to_string(size) + " bytes to file: " + this->path.string());
        }
        this->file.flush();
    }

    void FileOutput::clear()
    {
        this->file.close();
        std::filesystem::remove(this->path);
    }

    void FileOutput::check_open()
    {
        if (!this->file.is_open())
        {
            try
            {
                this->file.open(this->path, std::ios::binary);
            }
            catch (std::exception &e)
            {
                logger.error("Failed to open output file: " + this->path.string());
                logger.error(e.what());
                exit(1);
            }

            if (!this->file.is_open())
            {
                logger.error("Failed to open output file: " + this->path.string());
                exit(1);
            }
        }
    }
}