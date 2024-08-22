#include "logger.h"

Logger logger;

Logger::Logger()
{
}

// private
void Logger::log_time()
{
    std::cout << "\033[1;30m" << program_timer.elapsed_str() << "\033[0m";
}

// out
void Logger::log(std::string msg, LogLevel level)
{
    if (!settings::verbose || ((level == LogLevel::DEBUG || level == LogLevel::DEBUG_WARNING) && !settings::debug))
    {
        return;
    }

    // time print
    log_time();

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
    case LogLevel::DEBUG_WARNING:
        // light magenta
        std::cout << " [\033[1;35mDEBUG_WARN\033[0m]";
        break;
    default:
        break;
    }

    // message print
    std::cout << " " << msg << std::endl;
}

// in
std::string Logger::prompt(std::string msg)
{
    // time print
    log_time();

    std::cout << " [\033[1;34mIN\033[0m]";
    std::cout << " " << msg;

    std::string input;
    std::getline(std::cin, input);
    return input;
}

bool Logger::confirm_prompt(std::string msg)
{
    if (settings::no_confirm)
    {
        return true;
    }
    std::string input = prompt(msg + " [Y/n] ");

    std::transform(input.begin(), input.end(), input.begin(), ::tolower);
    return input == "y" || input == "yes";
}