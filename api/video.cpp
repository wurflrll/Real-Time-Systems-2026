extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include <iostream>
#include <vector>
#include <cstring>




struct FrameBuffer {
    uint32_t start_frame;
    uint32_t end_frame;
    std::vector<uint8_t> buffer;
};

typedef std::vector<FrameBuffer> FrameQueue;



class VideoFormat {


    private:

        AVFormatContext* fmtCtx = nullptr;
        AVCodecParameters* codecpar;
        AVCodecContext* codecCtx;

        AVPacket* pkt = av_packet_alloc();
        AVFrame* frame = av_frame_alloc();
        AVFrame* rgbFrame = av_frame_alloc();

        int height;
        int width;

        FrameQueue frame_queue;

        uint32_t start_frame;
        uint32_t end_frame;
        uint32_t frame_size;

    public:

    VideoFormat() {}

    bool InitialRead(char* filename, int start_second, int end_second);

    uint32_t GetTotalFrames();

    bool ProcessFrames(int number_frames);

    void AddHeader(uint8_t* buffer_ptr);
};




bool VideoFormat::InitialRead(char* filename, int start_second, int end_second) {
    
    vformat_network_init();

    AVFormatContext* fmtCtx = nullptr;
    avformat_open_input(&fmtCtx, filename, nullptr, nullptr);
    avformat_find_stream_info(fmtCtx, nullptr);

    int videoStream = -1;
    for (unsigned i = 0; i < fmtCtx->nb_streams; i++) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }

    codecpar = fmtCtx->streams[videoStream]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecCtx, codecpar);
    avcodec_open2(codecCtx, codec, nullptr);

    pkt = av_packet_alloc();
    frame = av_frame_alloc();
    rgbFrame = av_frame_alloc();

    width = codecCtx->width;
    height = codecCtx->height;

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 1);
    uint8_t* buffer = (uint8_t*)av_malloc(numBytes);

    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer,
                         AV_PIX_FMT_RGB24, width, height, 1);

    SwsContext* swsCtx = sws_getContext(
        width, height, codecCtx->pix_fmt,
        width, height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );


    AVRational time_base = fmtCtx->streams[videoStream]->time_base;
    int64_t initial_timestamp = 73 / av_q2d(time_base);
    // ---- SEEK TO 73 SECONDS ----
    int64_t timestamp = 73 * AV_TIME_BASE;
    std::cout << "av_seek status: " << av_seek_frame(fmtCtx, videoStream, initial_timestamp, AVSEEK_FLAG_BACKWARD);
    
    
    return true;
}
    
bool VideoFormat::ProcessFrames(int number_frames) {
    

    int frame_size = 54 + 3 * width * height;
    FrameBuffer new_buffer;
    new_buffer.buffer.reserve(number_frames * frame_size);

    new_buffer.start_frame = frame_queue[frame_queue.size() - 1].end_frame + 1;
    new_buffer.end_frame = new_buffer.start_frame + number_frames - 1;

    uint8_t buffer_ptr = new_buffer.buffer.data();

    int frame_count = 0;
    while (true) {

        int result = av_read_frame(fmtCtx, pkt);
        if (result < 0) {
            if (frame_count != number_frames) {
                std::cout << "A request has failed\n";
                return false;
            }
            return true;
        }

        std::cout << "start of a loop!\n";
        if (pkt->stream_index == videoStream) {
            avcodec_send_packet(codecCtx, pkt);
            while (avcodec_receive_frame(codecCtx, frame) == 0) {


                sws_scale(
                    swsCtx,
                    frame->data,
                    frame->linesize,
                    0,
                    height,
                    rgbFrame->data,
                    rgbFrame->linesize
                );


                memcpy((void*) (buffer_ptr + 54), rgbFrame->data[0], frame_size - 54);

                AddHeader(buffer_ptr);
               
                ++frame_count;
                if (frame_count > number_frames) {
                    return true;
                }
            } // potential issue seeing a frame and not using it
        }
        av_packet_unref(pkt);
    }
}


uint32_t VideoFormat::GetTotalFrames() {
    return (end_frame - start_frame) + 1;
}


void VideoFormat::AddHeader(uint8_t* buffer_ptr) {
    unsigned char bmpfileheader[14] = {
        'B','M',
        (unsigned char)(filesize),
        (unsigned char)(filesize >> 8),
        (unsigned char)(filesize >> 16),
        (unsigned char)(filesize >> 24),
        0,0,0,0,
        54,0,0,0
    };

    unsigned char bmpinfoheader[40] = {
        40,0,0,0,
        (unsigned char)(width),
        (unsigned char)(width >> 8),
        (unsigned char)(width >> 16),
        (unsigned char)(width >> 24),
        (unsigned char)(height),
        (unsigned char)(height >> 8),
        (unsigned char)(height >> 16),
        (unsigned char)(height >> 24),
        1,0,
        24,0
    };

    memcpy(buffer_ptr, (uint8_t*) bmpfileheader, 14);
    memcpy(buffer_ptr, (uint8_t*) bmpfileheader, 40);
}
