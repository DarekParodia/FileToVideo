#pragma once

#include <iostream>
#include <string>

#include "utils/timer.h"
#include "settings.h"

enum class LogLevel
{
    INFO,
    WARNING,
    ERROR,
    DEBUG,
    DEBUG_WARNING
};
typedef LogLevel LogLevel;

class Logger
{
public:
    Logger();

    // out
    void log(std::string msg, LogLevel level = LogLevel::INFO);
    void info(std::string msg) { log(msg, LogLevel::INFO); }
    void warning(std::string msg) { log(msg, LogLevel::WARNING); }
    void error(std::string msg) { log(msg, LogLevel::ERROR); }
    void debug(std::string msg) { log(msg, LogLevel::DEBUG); }
    void debug_warning(std::string msg) { log(msg, LogLevel::DEBUG_WARNING); }

    // in
    std::string prompt(std::string msg);
    bool confirm_prompt(std::string msg);
    bool continue_prompt() { return confirm_prompt("Continue?"); }

    // misc
    void flush() { std::cout << std::flush; }

private:
    void log_time();
};

// Global logger object
extern Logger logger;