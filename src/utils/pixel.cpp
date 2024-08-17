#include "pixel.h"

namespace utils
{
    pixel get_average_color(pixel *pixels, size_t size)
    { // this should be improved in the future
        size_t avg_r = 0;
        size_t avg_g = 0;
        size_t avg_b = 0;
        pixel average = {0, 0, 0};
        for (size_t i = 0; i < size; i++)
        {
            avg_r += pixels[i].r;
            avg_g += pixels[i].g;
            avg_b += pixels[i].b;
        }
        average.r = avg_r / size;
        average.g = avg_g / size;
        average.b = avg_b / size;

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

    std::string get_ansi_color(std::string string, pixel color)
    {
        return "\033[38;2;" + std::to_string(color.r) + ";" + std::to_string(color.g) + ";" + std::to_string(color.b) + "m" + string + "\033[0m";
    }
}