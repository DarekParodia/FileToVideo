#include "pixel.h"

namespace utils
{
    pixel get_average_color(pixel *pixels, size_t size)
    {
        pixel average = {0, 0, 0};
        for (size_t i = 0; i < size; i++)
        {
            average.r += pixels[i].r;
            average.g += pixels[i].g;
            average.b += pixels[i].b;
        }
        average.r /= size;
        average.g /= size;
        average.b /= size;
        return average;
    }

    pixel get_pixel_diff(pixel a, pixel b)
    {
        pixel diff = {0, 0, 0};
        diff.r = a.r - b.r;
        diff.g = a.g - b.g;
        diff.b = a.b - b.b;
        return diff;
    }

    pixel get_pixel_distances(pixel a, pixel b)
    {
        pixel distances = {0, 0, 0};
        distances.r = abs(a.r - b.r);
        distances.g = abs(a.g - b.g);
        distances.b = abs(a.b - b.b);
        return distances;
    }

    uint8_t get_pixel_distance(pixel a, pixel b)
    {
        pixel distances = get_pixel_distances(a, b);
        return (distances.r + distances.g + distances.b) / 3;
    }
}