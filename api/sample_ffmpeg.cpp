extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include <iostream>
#include <cstdio>
#include <string>

static void save_bmp(const std::string& filename, AVFrame* rgbFrame, int width, int height) {
    FILE* f = fopen(filename.c_str(), "wb");

    if (!f) {  
        std::cout << "file could not be saved\n";
    }

    int filesize = 54 + 3 * width * height;

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

    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);

    for (int y = height - 1; y >= 0; y--) {
        fwrite(rgbFrame->data[0] + y * rgbFrame->linesize[0], 1, width * 3, f);
    }

    fclose(f);
}

int main() {
    const char* filename = "input.mp4";

    avformat_network_init();

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

    AVCodecParameters* codecpar = fmtCtx->streams[videoStream]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecCtx, codecpar);
    avcodec_open2(codecCtx, codec, nullptr);

    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    AVFrame* rgbFrame = av_frame_alloc();

    int width = codecCtx->width;
    int height = codecCtx->height;

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

    int frameIndex = 0;

    // ---- READ UNTIL 76 SECONDS ----
    while (true) {

        int result = av_read_frame(fmtCtx, pkt);
        if (result < 0) {
            std::cout << "av read is over: " << result << "\n";
            break;
        }
        if (pkt->stream_index == videoStream) {
            avcodec_send_packet(codecCtx, pkt);
            while (avcodec_receive_frame(codecCtx, frame) == 0) {
                double pts_time = frame->best_effort_timestamp * av_q2d(time_base);


                if (pts_time < 73) continue;
                if (pts_time > 76) break;

                sws_scale(
                    swsCtx,
                    frame->data,
                    frame->linesize,
                    0,
                    height,
                    rgbFrame->data,
                    rgbFrame->linesize
                );

                char out[256];
                sprintf(out, "frame_%04d.bmp", frameIndex++);

                save_bmp(out, rgbFrame, width, height);
            }
        }
        av_packet_unref(pkt);
    }

    // cleanup
    av_free(buffer);
    av_frame_free(&frame);
    av_frame_free(&rgbFrame);
    av_packet_free(&pkt);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&fmtCtx);
    sws_freeContext(swsCtx);

    return 0;
}