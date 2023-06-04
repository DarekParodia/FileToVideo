#include "libattopng.h"
#include "generator.cpp"
#include "functions.cpp"
#include "vidgen.cpp"
#include <opencv2/opencv.hpp>
#include <cstdio>
#include <thread>
#include <ctime>
#include <vector>
#include <iostream>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

std::string output_filename = "output.mp4";
std::string output_dir = "./";
int width = 1920;
int height = 1080;
int scale = 0;
int fps = 30;
bool skip_continue = false;
bool decrypt_mode = false;

void encrypt(char *filename)
{
    std::cout << "Output filename: " << output_filename << std::endl;
    gen::generator *gena = new gen::generator(width, height, scale);
    int max_bytes = gena->getMaxBytes();
    std::cout << "Max bytes per frame: " << max_bytes << std::endl;
    std::cout << "Reading file \"" << filename << "\" size of: " << fn::getFileSize(filename) << " bytes, " << ceil((float)fn::getFileSize(filename) / (float)max_bytes) << " frames will be generated with framerate of: " << fps << " FPS" << std::endl;

    if (!skip_continue)
    {
        std::cout << "Continue? (y/n) ";
        char c;
        std::cin >> c;
        if (c != 'y')
            return;
    }
    else
        std::cout << "Skipping continue prompt" << std::endl;

    int number_of_frames = ceil((float)fn::getFileSize(filename) / (float)max_bytes);
    if (number_of_frames == 0)
        number_of_frames = 1;

    fs::remove_all("test");
    fs::remove_all("test2");
    fs::remove(output_dir + output_filename);

    std::cout << "Generating..." << std::endl;

    fn::ProgressBar bar(number_of_frames, "frames: ");
    vidgen::video *video = new vidgen::video(width, height, output_dir + output_filename, fps);
    for (int i = 0; i < number_of_frames; i++)
    {
        bar.update(i);
        char *data = fn::readFile(filename, max_bytes * i, max_bytes);
        libattopng_t *img = gena->generate(data);
        uint8_t *buffer = fn::getBuffer(img);
        video->writeByFrame(vidgen::frame(width, height, buffer, 1));
        libattopng_destroy(img);
    }
    video->release();
    bar.update(number_of_frames);
    std::cout << std::endl;
    std::cout << "Done!" << std::endl;
    // fn::generateVideo("test", argc, argv);
}
void decrypt(char *filename)
{
    std::cout << "Not Done yet.." << std::endl;
}
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg.find("-y") != std::string::npos)
        {
            skip_continue = true;
        }
        else if (arg.find("-o") != std::string::npos)
        {
            output_filename = argv[i + 1];
            if (output_filename.find(".mp4") == std::string::npos)
                output_filename += ".mp4";
        }
        else if (arg.find("-d") != std::string::npos)
        {
            decrypt_mode = true;
        }
        else if (arg.find("-w") != std::string::npos)
        {
            width = std::stoi(argv[i + 1]);
            if (width % 8 != 0)
            {
                std::cout << "Width must be divisible by 8" << std::endl;
                return 1;
            }
        }
        else if (arg.find("-h") != std::string::npos)
        {
            height = std::stoi(argv[i + 1]);
            if (height % 8 != 0)
            {
                std::cout << "Height must be divisible by 8" << std::endl;
                return 1;
            }
        }
        else if (arg.find("-s") != std::string::npos)
        {
            scale = std::stoi(argv[i + 1]);
        }
        else if (arg.find("-f") != std::string::npos)
        {
            fps = std::stoi(argv[i + 1]);
        }
    }
    if (decrypt_mode)
        decrypt(argv[1]);
    else
        encrypt(argv[1]);

    return 0;
}
