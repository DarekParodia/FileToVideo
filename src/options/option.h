#pragma once
#include <string>

class option
{
public:
    virtual ~option() = default;
    std::string getShortCall();
    std::string getLongCall();
    std::string getName();
    std::string getDescription();
    bool getHasArg();
    bool getRequired();
    bool getExecuted();
    virtual void execute(std::string arg) = 0;
    virtual void check() = 0;

protected:
    std::string shortcall;
    std::string longcall;
    std::string name;
    std::string description;
    bool hasArg = false;
    bool required = false;
    bool executed = false;
};