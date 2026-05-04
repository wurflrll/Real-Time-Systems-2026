#include "video.h"




const uint32_t max_seconds = 20;


void SendBuffer(const std::vector<uint8_t>& data, httplib::ws::WebSocket &ws) {
    uint32_t half = data.size() / 2;
    ws.send(reinterpret_cast<const char*>(data.data()), half);
    ws.send(reinterpret_cast<const char*>(data.data() + half), data.size() - half);
}

bool VideoFormat::InitialSetup(char* filename) {

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

    // added 54 here
    buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 1) + 54;


    std::cout << "NUM BYTES AV AV_IMAGE: " << buffer_size << "\n"; 


    buffer = (uint8_t*) av_malloc(buffer_size);

    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer + 54,
                         AV_PIX_FMT_RGB24, width, height, 1);

    swsCtx = sws_getContext(
        width, height, codecCtx->pix_fmt,
        width, height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    time_base = fmtCtx->streams[videoStream]->time_base;

    int64_t initial_timestamp = av_rescale_q(
        0, // replacing start second
        (AVRational){1, 1},
        fmtCtx->streams[videoStream]->time_base
    );

    std::cout << "av_seek status: " << av_seek_frame(fmtCtx, videoStream, initial_timestamp, AVSEEK_FLAG_BACKWARD);
   
    avcodec_flush_buffers(codecCtx);

    avformat_flush(fmtCtx);

    return true;
}

bool VideoFormat::InitialRead() {

    int videoStream = -1;
    for (unsigned i = 0; i < fmtCtx->nb_streams; i++) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }

    assert(videoStream != -1);

    int frame_count = 0;
    while (true) {

        int result = av_read_frame(fmtCtx, pkt);
        if (result < 0) {
            // if (frame_count != number_frames) {
            //     std::cout << "A request has failed\n";
            //     return false;
            // }
            std::cout << "FRAME COUNT: " << frame_count << "\n";

            return true;
        }

        // PRIMARY buffer with one allocation before compression:
        
        if (pkt->stream_index == videoStream) {
            avcodec_send_packet(codecCtx, pkt);
            while (avcodec_receive_frame(codecCtx, frame) == 0) {

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
                    return false;
                }
                
                PushFrame();

                double pts_time = frame->best_effort_timestamp * av_q2d(time_base);
                if ((double) max_seconds < pts_time) {
                    av_packet_unref(pkt);
                    return true;
                }
            } 
        }
        av_packet_unref(pkt);
    }
}



bool VideoFormat::PushFrame() {
    double pts_time = frame->best_effort_timestamp * av_q2d(time_base);

    AddHeader(buffer);

    std::vector<uint8_t> compressed = compressBuffer(buffer, buffer_size);

    CompressedFrame new_frame_buffer;

    new_frame_buffer.buffer_ptr = (uint8_t*) malloc(compressed.size());

    new_frame_buffer.buffer_size = compressed.size();

    new_frame_buffer.time_stamp = pts_time;

    memcpy(new_frame_buffer.buffer_ptr, compressed.data(), compressed.size());

    frame_array.push_back(new_frame_buffer);

    return true; // FIX return conditions
}


uint32_t VideoFormat::GetFrameIndex(double time_stamp) {

    uint32_t left = 0;
    uint32_t right = frame_array.size() - 1;

    while (1) {
        uint32_t middle = (left + right)/2;

        if (frame_array[middle].time_stamp > time_stamp) { 
            right = middle;
        }
        else { 
            left = middle;
        }
        if (right - left <= 1) {
            return left;
        }
    }
}

    
void VideoFormat::ProcessFrames(uint32_t& frame_index, uint32_t number_frames, httplib::ws::WebSocket &ws) {

    std::cout << "number of frames:" << number_frames << "\n";
    for (int i = 0; i < number_frames; ++i) { 
        if (frame_index < frame_array.size()) {
            CompressedFrame c_frame = frame_array[frame_index];
            uint32_t half = c_frame.buffer_size / 2;
            ws.send(reinterpret_cast<const char*>(c_frame.buffer_ptr), half);
            ws.send(reinterpret_cast<const char*>(c_frame.buffer_ptr + half), c_frame.buffer_size - half);
        }
        else { 
            break;
        }
        frame_index += 1;
    }
    std::cout << "done sending\n";
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
