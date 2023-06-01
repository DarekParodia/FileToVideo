#ifndef GENERATOR_CPP
#define GENERATOR_CPP 1
#include "functions.cpp"
#include "libattopng.h"
#include <math.h>
#include <iostream>
namespace gen
{
    class generator
    {
    public:
        generator(int width, int height, int scale = 0)
        {
            this->width = width;
            this->height = height;
            this->pixel_width = pow(2, scale);
            this->max_bytes = ((height * width) / 8) / this->pixel_width;
            std::cout << "Scale: " << scale << std::endl;
            std::cout << "Width: " << width << std::endl;
            std::cout << "Height: " << height << std::endl;
        };
        libattopng_t *generate(char *data)
        {
            int byte_counter = 0;
            libattopng_t *png = libattopng_new(width / this->pixel_width, height / this->pixel_width, PNG_GRAYSCALE);
            //
            int x, y;
            for (y = 0; y < this->height / this->pixel_width; y++)
            {
                for (x = 0; x < this->width / this->pixel_width;)
                {
                    for (int i = 0; i < 8; i++)
                    {
                        libattopng_set_pixel(png, x, y, 0);
                        if (fn::getBit(data[byte_counter], i))
                            libattopng_set_pixel(png, x, y, 255);
                        if (x >= this->width / this->pixel_width)
                            break;
                        else
                            x++;
                    }
                    byte_counter++;
                }
                // bar.update(y + 1);
            }
            // std::cout << std::endl;
            if (this->pixel_width > 1)
                return fn::scalePng(png, this->pixel_width);
            else
                return png;
        };
        int getMaxBytes()
        {
            return this->max_bytes;
        };

    private:
        int width = 512;
        int height = 512;
        int max_bytes = 512;
        int pixel_width = 1;
    };
}
#endif