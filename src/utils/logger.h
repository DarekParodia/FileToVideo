#pragma once
#include <iostream>

#include "utils/timer.h"

enum class LogLevel
{
    INFO,
    WARNING,
    ERROR
};
typedef LogLevel LogLevel;

class Logger
{
public:
    Logger();
    void log(std::string msg, LogLevel level = LogLevel::INFO);
    void info(std::string msg) { log(msg, LogLevel::INFO); }
    void warning(std::string msg) { log(msg, LogLevel::WARNING); }
    void error(std::string msg) { log(msg, LogLevel::ERROR); }
};

// Global logger object
extern Logger logger;