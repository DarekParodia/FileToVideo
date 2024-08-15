#pragma once

#include <vector>
#include "settings.h"
#include "option.h"
#include "utils/logger.h"
#include "utils/general.h"

namespace arguments
{
    class input_file : public option
    {
    public:
        input_file();
        ~input_file() override;
        void execute(std::string arg) override;
        void check() override;
    };

    class output_file : public option
    {
    public:
        output_file();
        ~output_file() override;
        void execute(std::string arg) override;
        void check() override;
    };

    class ffmpeg_path : public option
    {
    public:
        ffmpeg_path();
        ~ffmpeg_path() override;
        void execute(std::string arg) override;
        void check() override;
    };

    class debug : public option
    {
    public:
        debug();
        ~debug() override;
        void execute(std::string arg) override;
        void check() override;
    };

    class decode : public option
    {
    public:
        decode();
        ~decode() override;
        void execute(std::string arg) override;
        void check() override;
    };

    class width : public option
    {
    public:
        width();
        ~width() override;
        void execute(std::string arg) override;
        void check() override;
    };

    class height : public option
    {
    public:
        height();
        ~height() override;
        void execute(std::string arg) override;
        void check() override;
    };

    class fps : public option
    {
    public:
        fps();
        ~fps() override;
        void execute(std::string arg) override;
        void check() override;
    };

    class pixel_size : public option
    {
    public:
        pixel_size();
        ~pixel_size() override;
        void execute(std::string arg) override;
        void check() override;
    };

    class use_color : public option
    {
    public:
        use_color();
        ~use_color() override;
        void execute(std::string arg) override;
        void check() override;
    };

    // Global argument list
    extern std::vector<option *> defined_options;

}