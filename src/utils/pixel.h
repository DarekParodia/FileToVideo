#pragma once

#include <cstdint>
#include <cstdlib>

namespace utils
{
    struct pixel
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };
    typedef struct pixel pixel;

    pixel get_average_color(pixel *pixels, size_t size);
    pixel get_pixel_diff(pixel a, pixel b);
    pixel get_pixel_distances(pixel a, pixel b);
    uint8_t get_pixel_distance(pixel a, pixel b);

}