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



typedef std::vector<uint8_t> Frame;



class VideoBuffer {

    public: 

    uint32_t frame_size;
    std::vector<Frame> frame_buffers;

    VideoBuffer() {}

    void AllocateFrames(uint32_t number_frames);

    uint32_t GetFrame(uint32_t index);
};


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

        uint32_t start_frame;
        uint32_t end_frame;

        uint8_t* buffer;

    public:

        uint32_t frame_size;

    VideoFormat() {}

    bool InitialRead(char* filename, uint32_t start_second);

    uint32_t GetTotalFrames();

    bool ProcessFrames(uint32_t number_frames, VideoBuffer& video_buffer, httplib::ws::WebSocket &ws);

    void AddHeader(uint8_t* buffer_ptr);
};

void SendBuffer(const std::vector<uint8_t>& data, httplib::ws::WebSocket &ws);
