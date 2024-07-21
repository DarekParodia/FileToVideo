#include "general.h"

// char array to string
std::string char_array_to_string(char **arr, int size)
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