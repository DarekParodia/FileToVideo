#include "libattopng.h"
#include "generator.cpp"
#include "functions.cpp"
#include <opencv2/opencv.hpp>
#include <cstdio>
#include <thread>
#include <ctime>
#include <vector>
#include <iostream>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;
int width = 1920;
int height = 1080;
bool skip_continue = false;

void encrypt(char *filename, int argc, char *argv[])
{
    gen::generator *gena = new gen::generator(width, height, 0);
    int max_bytes = gena->getMaxBytes();
    std::cout << "Max bytes  per frame: " << max_bytes << std::endl;
    std::cout << "Reading file \"" << argv[1] << "\" size of: " << fn::getFileSize(filename) << " bytes " << std::endl;
    if (!skip_continue)
    {
        std::cout << "Continue? (y/n) ";
        char c;
        std::cin >> c;
        if (c != 'y')
            return;
    }
    int number_of_frames = ceil((float)fn::getFileSize(filename) / (float)max_bytes);
    if (number_of_frames == 0)
        number_of_frames = 1;
    std::cout << "Number of frames: " << number_of_frames << std::endl;

    fs::remove_all("test");
    fs::remove_all("test2");
    fs::remove("output.mp4");
    fs::create_directory("test");
    fs::create_directory("test2");

    std::cout << "Generating..." << std::endl;

    cv::VideoWriter writer("output.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 5, cv::Size(width, height), true);
    if (!writer.isOpened())
    {
        std::cout << "Could not open the output video for write: " << std::endl;
        return;
    }

    fn::ProgressBar bar(number_of_frames, "frames: ");

    for (int i = 0; i < number_of_frames; i++)
    {
        bar.update(i);
        char *data = fn::readFile(filename, max_bytes * i, max_bytes);
        libattopng_t *img = gena->generate(data);

        // size_t len;
        // char *data2 = libattopng_get_data(img, &len);
        // cv::Mat frame(height, width, CV_8UC(1), data2);

        uint8_t *buffer = fn::getBuffer(img);
        // fn::printHex(buffer, width * height);
        // cv::Mat frame = fn::createFrame(width, height, buffer);
        cv::Mat frame(height, width, CV_8UC1, buffer);
        cv::imwrite("test2/" + std::to_string(i) + ".png", frame);
        writer.write(frame);
        fn::savePng(fn::generateFromBuffer(buffer, width, height), "test/" + std::to_string(i) + ".png");
        libattopng_destroy(img);
    }
    bar.update(number_of_frames);
    writer.release();
    // fn::generateVideo("test", argc, argv);
}
void decrypt(char *filename, int argc, char *argv[])
{
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
            std::cout << "Skipping continue prompt" << std::endl;
        }
    }
    encrypt(argv[1], argc, argv);
    return 0;
}
