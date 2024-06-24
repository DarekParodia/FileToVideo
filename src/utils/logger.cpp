#include "logger.h"

Logger logger;

Logger::Logger()
{
}

void Logger::log(std::string msg, LogLevel level)
{
    // time print
    std::cout << "\033[1;30m" << program_timer.elapsed_str() << "\033[0m";

    // status print
    switch (level)
    {
    case LogLevel::INFO:
        std::cout << " [\033[1;32mINFO\033[0m]";
        break;
    case LogLevel::WARNING:
        std::cout << " [\033[1;33mWARN\033[0m]";
        break;
    case LogLevel::ERROR:
        std::cout << " [\033[1;31mERROR\033[0m]";
        break;
    default:
        break;
    }

    // message print
    std::cout << " " << msg << std::endl;
}