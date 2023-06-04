#ifndef VIDGEN
#define VIDGEN 1
#include <iostream>
#include <opencv2/opencv.hpp>
#include "functions.cpp"
namespace vidgen
{
    struct frame
    {
        int bytesPerPixel;
        int width;
        int height;
        uint8_t *buffer;
        frame(int width, int height, uint8_t *buffer, int bytesPerPixel)
        {
            this->width = width;
            this->height = height;
            this->bytesPerPixel = bytesPerPixel;
            this->buffer = buffer;
        }
    };
    class video
    {
    public:
        video(int width, int height, std::string filename, double fps = 30, int fourcc = cv::VideoWriter::fourcc('H', '2', '6', '4'))
        {
            this->width = width;
            this->height = height;
            this->fps = fps;
            this->filename = filename;
            this->fourcc = fourcc;
            this->writer = cv::VideoWriter(filename, fourcc, fps, cv::Size(width, height), false);
        }
        void addFrame(frame frame)
        {
            frames.push_back(frame);
        }
        void writeByFrame(frame frame)
        {
            if (!writer.isOpened())
            {
                std::cout << "Error: writer is not opened" << std::endl;
                return;
            }
            cv::Mat mat(height, width, CV_8UC(frame.bytesPerPixel), frame.buffer);
            if (mat.empty())
            {
                std::cout << "Error: mat is empty" << std::endl;
                return;
            }
            cv::Mat binary;
            cv::threshold(mat, binary, 127, 255, cv::THRESH_BINARY);
            writer.write(binary);
        }
        void release()
        {
            writer.release();
        }
        void write()
        {
            if (!writer.isOpened())
            {
                std::cout << "Error: writer is not opened" << std::endl;
                return;
            }
            std::string backendName = writer.getBackendName();
            std::cout << "Using backend: " << backendName << std::endl;
            fn::ProgressBar bar(frames.size(), "frames: ");
            for (int i = 0; i < frames.size(); i++)
            {
                cv::Mat mat(height, width, CV_8UC(frames[i].bytesPerPixel), frames[i].buffer);
                if (mat.empty())
                {
                    std::cout << "Error: mat is empty" << std::endl;
                    break;
                }
                writer.write(mat);
                bar.update(i);
            }
            writer.release();
            bar.update(frames.size());
        }

    private:
        int width;
        int height;
        int fourcc;
        double fps;
        cv::VideoWriter writer;
        std::string filename;
        std::vector<frame> frames;
    };
}
#endif