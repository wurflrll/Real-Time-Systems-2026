#include "video.h"


void SendBuffer(const std::vector<uint8_t>& data, httplib::ws::WebSocket &ws) {
    uint32_t half = data.size() / 2;
    ws.send(reinterpret_cast<const char*>(data.data()), half);
    ws.send(reinterpret_cast<const char*>(data.data() + half), data.size() - half);
}

bool VideoFormat::InitialRead(char* filename, uint32_t start_second) {

    avformat_network_init();

    fmtCtx = nullptr;
    if (avformat_open_input(&fmtCtx, filename, nullptr, nullptr) < 0) {
        std::cout << "file read failed in lib\n";
        return false;
    };
    if (fmtCtx == nullptr) {
        std::cout << "fmtCtx couldn't be allocated\n";
        return false;
    }
    avformat_find_stream_info(fmtCtx, nullptr);

    int videoStream = -1;
    for (unsigned i = 0; i < fmtCtx->nb_streams; i++) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }

    if (videoStream == -1) {
        std::cout << "a stream could not be found\n";
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

    frame_size = height * width * 3 + 54;


    // added 54 here
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 1) + 54;

    std::cout << "FRAME SIZE INITIAL READ: " << frame_size << "\n";

    std::cout << "NUM BYTES AV AV_IMAGE: " << numBytes << "\n"; 


    buffer = (uint8_t*) av_malloc(numBytes);

    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer + 54,
                         AV_PIX_FMT_RGB24, width, height, 1);

    swsCtx = sws_getContext(
        width, height, codecCtx->pix_fmt,
        width, height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    AVRational time_base = fmtCtx->streams[videoStream]->time_base;

    int64_t initial_timestamp = av_rescale_q(
        start_second,
        (AVRational){1, 1},
        fmtCtx->streams[videoStream]->time_base
    );
    std::cout << "av_seek status: " << av_seek_frame(fmtCtx, videoStream, initial_timestamp, AVSEEK_FLAG_BACKWARD);
   
    avcodec_flush_buffers(codecCtx);

    avformat_flush(fmtCtx);

    return true;
}



    
bool VideoFormat::ProcessFrames(uint32_t number_frames, VideoBuffer& video_buffer, httplib::ws::WebSocket &ws) {

    int videoStream = -1;
    for (unsigned i = 0; i < fmtCtx->nb_streams; i++) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }

    uint8_t* one_time_buffer = (uint8_t*) malloc(frame_size);

    video_buffer.frame_buffers.resize(number_frames);

    int frame_count = 0;
    while (true) {

        int result = av_read_frame(fmtCtx, pkt);
        if (result < 0) {
            if (frame_count != number_frames) {
                std::cout << "A request has failed\n";
                free(one_time_buffer);
                return false;
            }
            return true;
        }

        // PRIMARY buffer with one allocation before compression:
        
        if (pkt->stream_index == videoStream) {
            avcodec_send_packet(codecCtx, pkt);
            while (avcodec_receive_frame(codecCtx, frame) == 0) {

                uint8_t* buffer_ptr = one_time_buffer;

                // allocate a new buffer here:
                
                int scale_ret = sws_scale(
                    swsCtx,
                    frame->data,
                    frame->linesize,
                    0,
                    height,
                    rgbFrame->data,
                    rgbFrame->linesize
                );

                if (scale_ret < 0) { 
                    std::cout << "sws failed\n";
                    free(one_time_buffer);
                    return false;
                }
            
                // uint32_t byte_location = 54; 
                
                // for (int y = height - 1; y >= 0; y--) {
                //     memcpy(buffer_ptr + byte_location, rgbFrame->data[0] + y * rgbFrame->linesize[0], width * 3);
                //     byte_location += width * 3;
                //     std::cout << "width: " << width * 3 <<"\n";
                //     std::cout << "linesize: " << rgbFrame->linesize[0] << "\n";
                // }
                
            

                AddHeader(buffer);
                
                SendBuffer(compressBuffer(buffer, frame_size), ws);


                ++frame_count;
                if (frame_count >= number_frames) {
                    av_packet_unref(pkt);
                    free(one_time_buffer);
                    return true;
                }
            } 
        }
        av_packet_unref(pkt);
    }
}


uint32_t VideoFormat::GetTotalFrames() {
    return 24;
}


void VideoFormat::AddHeader(uint8_t* buffer_ptr) {

    int file_size = 54 + 3 * width * height;
    unsigned char bmpfileheader[14] = {
        'B','M',
        (unsigned char)(file_size),
        (unsigned char)(file_size >> 8),
        (unsigned char)(file_size >> 16),
        (unsigned char)(file_size >> 24),
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
    memcpy(buffer_ptr + 14, (uint8_t*) bmpinfoheader, 40);
}
