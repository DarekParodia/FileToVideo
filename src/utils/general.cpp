#include "general.h"

// char array to string
std::string char_array_to_string(const char **arr, int size)
{
    std::string str;
    for (int i = 0; i < size; i++)
    {
        if (i > 0)
            str += ", ";
        str += '[';
        str += arr[i];
        str += ']';
    }
    return str;
}

std::string get_arg_value(std::string arg)
{
    std::string value;
    size_t pos = arg.find("=");
    if (pos != std::string::npos)
    {
        value = arg.substr(pos + 1);
    }
    else
    {
        value = "";
    }
    return value;
}

std::string get_arg_key(std::string arg)
{
    std::string key;
    size_t pos = arg.find("=");
    if (pos != std::string::npos)
    {
        key = arg.substr(0, pos);
    }
    else
    {
        key = arg;
    }
    return key;
}

std::string format_bytes(unsigned long bytes)
{
    double bytes_db = (double)bytes;
    std::string suffix = "B";
    if (bytes_db >= 1024)
    {
        suffix = "KB";
        bytes_db /= 1024;
    }
    if (bytes_db >= 1024)
    {
        suffix = "MB";
        bytes_db /= 1024;
    }
    if (bytes_db >= 1024)
    {
        suffix = "GB";
        bytes_db /= 1024;
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3) << bytes_db;
    return oss.str() + suffix;
}

std::string bytes_to_hex_string(const uint8_t *bytes, size_t size)
{
    std::stringstream ss;
    for (size_t i = 0; i < size; ++i)
    {
        if (i > 0)
            ss << " ";
        ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i];
    }
    return ss.str();
}

std::string bytes_to_bit_string(const uint8_t *bytes, size_t size)
{
    std::stringstream ss;
    for (size_t i = 0; i < size; ++i)
    {
        if (i > 0)
            ss << " ";
        for (int j = 7; j >= 0; j--)
        {
            ss << ((bytes[i] >> j) & 1);
        }
    }
    return ss.str();
}