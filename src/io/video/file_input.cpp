#include "file_input.h"

namespace io::video
{
    FileInput::FileInput(std::string filename)
    {
        this->filename = filename;
    }

    FileInput::~FileInput()
    {
        close();
    }

    void FileInput::open()
    {
        if (this->is_open)
        {
            logger.debug_warning("Video file is already open");
            return;
        }

        // Open video file
        if (avformat_open_input(&pFormatCtx, filename.c_str(), nullptr, nullptr) != 0)
        {
            logger.error("Could not open file: " + filename);
            exit(1);
        }

        // Retrieve stream information
        if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
        {
            logger.error("Could not find stream information");
            exit(1);
        }

        // Find the first video stream
        videoStreamIndex = -1;
        for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++)
        {
            if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                videoStreamIndex = i;
                break;
            }
        }

        if (videoStreamIndex == -1)
        {
            logger.error("Could not find video stream");
            exit(1);
        }

        // Get codec parameters
        auto codecParameters = pFormatCtx->streams[videoStreamIndex]->codecpar;

        // Find the decoder for the video stream
        pCodec = const_cast<AVCodec *>(avcodec_find_decoder(codecParameters->codec_id));
        if (pCodec == nullptr)
        {
            logger.error("Unsupported codec!");
            exit(1);
        }

        // Allocate a codec context for the decoder
        pCodecCtx = avcodec_alloc_context3(pCodec);
        if (avcodec_parameters_to_context(pCodecCtx, codecParameters) < 0)
        {
            logger.error("Could not copy codec parameters to codec context");
            exit(1);
        }

        // Open codec
        if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0)
        {
            logger.error("Could not open codec");
            exit(1);
        }

        // Allocate video frame
        pFrameRGB = av_frame_alloc();
        if (pFrameRGB == nullptr)
        {
            logger.error("Could not allocate video frame");
            exit(1);
        }

        numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);
        buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
        av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);

        // Initialize SWS context for software scaling
        sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);

        if (sws_ctx == nullptr)
        {
            logger.error("Could not initialize the conversion context");
            exit(1);
        }

        // Allocate video frame
        this->pFrame = av_frame_alloc();

        this->is_open = true;
        logger.debug("Opened video file: " + filename);
    }

    void FileInput::close()
    {
        // Cleanup
        av_frame_free(&pFrame);
        av_frame_free(&pFrameRGB);
        avcodec_free_context(&pCodecCtx);
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        av_freep(&buffer);
        sws_freeContext(sws_ctx);

        this->is_open = false;

        logger.debug("Closed video file: " + filename);
    }

    void FileInput::update()
    {
        // Clear any previous errors
        av_packet_unref(&packet);

        // Read exactly one frame
        while (av_read_frame(pFormatCtx, &packet) >= 0)
        {
            if (packet.stream_index == videoStreamIndex)
            {
                int response = avcodec_send_packet(pCodecCtx, &packet);
                if (response < 0)
                {
                    logger.error("Failed to send packet to codec");
                    break;
                }

                response = avcodec_receive_frame(pCodecCtx, pFrame);
                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
                {
                    continue;
                }
                else if (response < 0)
                {
                    logger.error("Failed to receive frame from codec");
                    break;
                }

                sws_scale(sws_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

                // Ensure buffer is properly managed before pushing
                if (buffer)
                {
                    uint8_t *new_buffer = new uint8_t[numBytes];
                    memcpy(new_buffer, buffer, numBytes);
                    this->frame_buffer.push(new_buffer);
                }

                // Free the packet that was allocated by av_read_frame
                av_packet_unref(&packet);
                break;
            }
        }
    }

    uint8_t *FileInput::readFrame()
    {
        // get the next frame from queue
        if (this->frame_buffer.empty())
        {
            this->update();
            if (this->frame_buffer.empty())
            {
                return nullptr;
            }
        }

        // get the frame
        logger.debug("Frame buffer size: " + std::to_string(this->frame_buffer.size()));
        uint8_t *frame = this->frame_buffer.front();
        this->frame_buffer.pop();
        logger.debug("Frame buffer size after: " + std::to_string(this->frame_buffer.size()));
        return frame;
    }
}
