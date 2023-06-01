#ifndef FUNCTIONS
#define FUNCTIONS 1
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <cstdio>
#include <opencv2/opencv.hpp>
#include "libattopng.h"
namespace fn
{
    class ProgressBar
    {
    public:
        ProgressBar(int total, char *message = "") : total_(total), message(message)
        {
            update(0);
        }

        inline void update(int progress)
        {
            float percent = (float)progress / total_;
            if (lastPercent != percent * 100)
            {
                int barWidth = 70;
                std::cout << "[";
                int pos = barWidth * percent;
                for (int j = 0; j < barWidth; j++)
                {
                    if (j < pos)
                        std::cout << "=";
                    else if (j == pos)
                        std::cout << ">";
                    else
                        std::cout << " ";
                }
                std::cout.flush();
                std::cout << "] " << message << progress << "/" << total_ << " " << int(percent * 100.0) << " %\r";
                lastPercent = percent * 100;
            }
        }

    private:
        int total_;
        char *message;
        int lastPercent = 0;
    };
    inline uint32_t rgbToUInt32(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
    {
        return (r << 24) | (g << 16) | (b << 8) | a;
    }
    inline libattopng_t *scalePng(libattopng_t *png, int scale)
    {
        libattopng_t *scaled = libattopng_new(png->width * scale, png->height * scale, PNG_GRAYSCALE);
        for (int y = 0; y < png->height; y++)
        {
            for (int x = 0; x < png->width; x++)
            {
                uint32_t pixel = libattopng_get_pixel(png, x, y);
                for (int sy = 0; sy < scale; sy++)
                {
                    for (int sx = 0; sx < scale; sx++)
                    {
                        libattopng_set_pixel(scaled, x * scale + sx, y * scale + sy, pixel);
                    }
                }
            }
        }
        return scaled;
    }
    inline void savePng(libattopng_t *png, std::string filename)
    {
        libattopng_save(png, filename.c_str());
    }
    inline bool getBit(char c, int index)
    {
        char shifted = c >> index;
        char masked = shifted & 1;
        return (bool)masked;
    }
    inline void printBits(char c)
    {
        for (int i = 7; i >= 0; i--)
        {
            bool bitValue = (c & (1 << i)) ? 1 : 0;
            std::cout << bitValue;
        }
    }
    inline char *readFile(char *filename, int startingByte, int bytesToRead)
    {
        FILE *file = fopen(filename, "rb");
        fseek(file, startingByte, SEEK_SET);
        char *buffer = new char[bytesToRead];
        fread(buffer, 1, bytesToRead, file);
        fclose(file);
        return buffer;
    }
    inline long getFileSize(char *filename)
    {
        FILE *file = fopen(filename, "rb");
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fclose(file);
        return size;
    }
    inline void generateVideo(char *dir, int argc, char *argv[])
    {
        std::cout << "Generating video..." << std::endl;
        std::string command = "ffmpeg -framerate 60 -i " + std::string(dir) + "/%d.png -c:v libx264 -r 60 -pix_fmt yuv420p outputFFMPEG.avi -y";
        std::cout << command << std::endl;
        system(command.c_str());
        std::cout << "Done!" << std::endl;
    }
    inline cv::Mat createFrame(int width, int height, const uint8_t *pixels)
    {
        cv::Mat frame(height, width, CV_8UC(1));
        std::memcpy(frame.data, pixels, width * height);
        return frame;
    }
    inline uint8_t *getBuffer(libattopng_t *img, int bytesPerPixel = 1)
    {
        uint8_t *buffer = new uint8_t[img->width * img->height * bytesPerPixel];
        int counter = 0;
        for (size_t y = 0; y < img->height; y++)
        {
            for (size_t x = 0; x < img->width; x++)
            {
                buffer[counter] = ((libattopng_get_pixel(img, x, y) >> 0) & 0xff);
                counter++;
            }
        }

        return buffer;
    }
    inline libattopng_t *generateFromBuffer(uint8_t *buffer, int width, int height)
    {
        libattopng_t *img = libattopng_new(width, height, PNG_GRAYSCALE);
        int counter = 0;
        for (size_t y = 0; y < height; y++)
        {
            for (size_t x = 0; x < width; x++)
            {
                libattopng_set_pixel(img, x, y, buffer[counter]);
                counter++;
            }
        }
        return img;
    }
    inline void printHex(uint8_t *value, int lenght)
    {
        for (size_t i = 0; i < lenght; i++)
        {
            std::cout << " 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)value[i];
        }
        std::cout << std::endl;
    }
}
#endif