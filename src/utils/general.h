#pragma once

#include <cstdint>
#include <string>
#include <iomanip>
#include <sstream>

// version struct
typedef struct version
{
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
} version;

// char array to string
std::string char_array_to_string(const char **arr, int size);

std::string get_arg_value(std::string arg);
std::string get_arg_key(std::string arg);

std::string format_bytes(unsigned long bytes);

std::string bytes_to_hex_string(const uint8_t *bytes, size_t size);
