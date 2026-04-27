#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}


#include <iostream>
#include <vector>
#include <cstring>
#include "compress.hpp"




class VideoBuffer {

    public: 

    uint32_t frame_size;
    std::vector<uint8_t*> frame_buffers;

    VideoBuffer() {}

    void AllocateFrames(uint32_t number_frames);

    uint32_t GetFrame(uint32_t index);
};


struct FrameBuffer {
    uint32_t start_frame;
    uint32_t end_frame;
    std::vector<uint8_t*> buffer;
};

typedef std::vector<FrameBuffer> FrameQueue;



class VideoFormat {


    private:

        AVFormatContext* fmtCtx = nullptr;
        AVCodecParameters* codecpar;
        AVCodecContext* codecCtx;
        SwsContext* swsCtx;

        AVPacket* pkt = av_packet_alloc();
        AVFrame* frame = av_frame_alloc();
        AVFrame* rgbFrame = av_frame_alloc();

        int height;
        int width;

        FrameQueue frame_queue;

        uint32_t start_frame;
        uint32_t end_frame;

    public:

        uint32_t frame_size;

    VideoFormat() {}

    bool InitialRead(char* filename, uint32_t start_second);

    uint32_t GetTotalFrames();

    bool ProcessFrames(uint32_t number_frames, VideoBuffer& video_buffer);

    void AddHeader(uint8_t* buffer_ptr);
};
