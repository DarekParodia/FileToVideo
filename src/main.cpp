#include <iostream>
#include <string>

#include "settings.h"
#include "utils/timer.h"
#include "utils/logger.h"
#include "utils/general.h"
#include "options/arguments.h"
#include "generator/generator.h"
#include "decoder/decoder.h"

int handle_arguments(int argc, const char *argv[])
{
    // debug
    logger.debug("Parsing arguments");
    logger.debug("Arguments count: " + std::to_string(argc));
    logger.debug("Arguments: " + char_array_to_string(argv, argc));

    logger.debug("Option count: " + std::to_string(arguments::defined_options.size()));
    std::string options_str = "";
    for (int i = 0; i < arguments::defined_options.size(); i++)
    {
        if (i > 0)
            options_str += ", ";
        options_str += arguments::defined_options[i]->getName();
    }
    logger.debug("Options: " + options_str);

    // parse user arguments
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        std::string key = get_arg_key(arg);
        logger.debug("Parsing argument: " + arg + " with key: " + key);
        // check for options
        for (int j = 0; j < arguments::defined_options.size(); j++)
        {
            logger.debug("Checking option: " + arguments::defined_options[j]->getName());
            if (arguments::defined_options[j]->getShortCall() == key || arguments::defined_options[j]->getLongCall() == key)
            {
                arguments::defined_options[j]->execute(arg);
            }
        }
    }

    // check if all required options are set
    for (int i = 0; i < arguments::defined_options.size(); i++)
    {
        arguments::defined_options[i]->check();
    }
    return 0;
}

int main(int argc, const char *argv[])
{
    settings::program_name = std::string(argv[0]);
    logger.info("Starting FileToVideo Version " + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR) + "." + std::to_string(VERSION_PATCH));
    handle_arguments(argc, argv);

    if (!settings::decode)
    {
        // calculate requiraments and prompt user
        gen.calculate_requiraments();

        if (!logger.continue_prompt())
        {
            logger.warning("Exiting...");
            logger.flush();
            return 0;
        };

        // generate video
        gen.generate();
    }
    else
    {
        logger.info("Decoding video");

        io::video::VideoInput *input;
        input = new io::video::FileInput(settings::input_file_path);

        dec.setInputFile(input); // set video input to be sourced from a file
        dec.calculate_requiraments();

        if (!logger.continue_prompt())
        {
            logger.warning("Exiting...");
            logger.flush();
            return 0;
        };

        dec.decode();
    }
    logger.info("Done");
    logger.flush();
    return 0;
}
