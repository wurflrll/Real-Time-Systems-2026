#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}


#include "cpp-httplib/httplib.h"

#include <iostream>
#include <vector>
#include <cstring>
#include "compress.h"


struct CompressedFrame { 
    uint8_t* buffer_ptr;
    uint32_t buffer_size;
    double time_stamp;
};

class VideoFormat {


    private:

        AVFormatContext* fmtCtx = nullptr;
        AVCodecParameters* codecpar;
        AVCodecContext* codecCtx;
        SwsContext* swsCtx;

        AVRational time_base;

        AVPacket* pkt = av_packet_alloc();
        AVFrame* frame = av_frame_alloc();
        AVFrame* rgbFrame = av_frame_alloc();

        int height;
        int width;

        uint32_t start_frame;
        uint32_t end_frame;

        uint8_t* buffer;
        uint32_t buffer_size;

        std::vector<CompressedFrame> frame_array;

    public:

    VideoFormat() {}

    VideoFormat(char* filename) {
        std::cout << "Starting encoding process...." << "\n";
        if (!InitialSetup(filename)) { 
            std::cout << "Something went wrong in initial process\n";
        }
        InitialRead();
        std::cout << "Finished encoding\n";
    }

    bool InitialSetup(char* filename);

    bool InitialRead();

    bool PushFrame();

    uint32_t GetFrameIndex(double time_stamp);

    void ProcessFrames(uint32_t& frame_index, uint32_t number_frames, httplib::ws::WebSocket &ws);

    void AddHeader(uint8_t* buffer_ptr);
};

void SendBuffer(const std::vector<uint8_t>& data, httplib::ws::WebSocket &ws);
