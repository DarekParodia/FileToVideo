#pragma once

#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>

class timer
{
public:
    timer();
    void start();
    void reset();
    double elapsed() const;
    std::string elapsed_str() const;

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
};

// Global timer object
extern timer program_timer;