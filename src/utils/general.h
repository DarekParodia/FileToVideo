#pragma once

#include <cstdint>
#include <string>
#include <iomanip>
#include <sstream>

// char array to string
std::string char_array_to_string(const char **arr, int size);

std::string get_arg_value(std::string arg);
std::string get_arg_key(std::string arg);

std::string format_bytes(unsigned long bytes);

std::string bytes_to_hex_string(const uint8_t *bytes, size_t size);
