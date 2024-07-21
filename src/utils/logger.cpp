#include "logger.h"

Logger logger;

Logger::Logger()
{
}

void Logger::log(std::string msg, LogLevel level)
{
    if (!settings::verbose || (level == LogLevel::DEBUG && !settings::debug))
    {
        return;
    }

    // time print
    std::cout << "\033[1;30m" << program_timer.elapsed_str() << "\033[0m";

    switch (level)
    {
    case LogLevel::INFO:
        // green
        std::cout << " [\033[1;32mINFO\033[0m]";
        break;
    case LogLevel::WARNING:
        // yellow
        std::cout << " [\033[1;33mWARN\033[0m]";
        break;
    case LogLevel::ERROR:
        // light red
        std::cout << " [\033[1;31mERROR\033[0m]";
        break;
    case LogLevel::DEBUG:
        // light blue
        std::cout << " [\033[1;36mDEBUG\033[0m]";
        break;
    default:
        break;
    }

    // message print
    std::cout << " " << msg << std::endl;
}