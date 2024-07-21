#include "option.h"

std::string option::getShortCall()
{
    return shortcall;
}

std::string option::getLongCall()
{
    return longcall;
}

std::string option::getName()
{
    return name;
}

std::string option::getDescription()
{
    return description;
}

bool option::getHasArg()
{
    return hasArg;
}

bool option::getRequired()
{
    return required;
}

bool option::getExecuted()
{
    return executed;
}