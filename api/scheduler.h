#pragma once

#include <iostream>
#include <vector>
#include <mutex>
#include <chrono>
#include <thread>
#include <atomic>

#include "video.h"

#include "cpp-httplib/httplib.h"


struct RequestInfo { 
    uint32_t start_second;
    uint32_t number_frames;
    uint32_t movie_index;

    friend std::ostream& operator<<(std::ostream& os, const RequestInfo& req);
};



class Connection {
    private:
    
    RequestInfo request_info;
    httplib::ws::WebSocket* socket;
    uint32_t frame_index;
    VideoFormat* video_format;

    public:

    uint32_t frames_sent = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    bool initialized = false;
    std::atomic<bool> finished{false};
    uint32_t id;

    Connection() { }
    Connection(RequestInfo req, httplib::ws::WebSocket& ws) { 
        id = rand() % 100000;
        request_info = req;
        socket = &ws;
        start_time = std::chrono::high_resolution_clock::now();
    }
    bool GetFrames(uint32_t num_frames);
    void InitialWork();
};


class Scheduler {

    private:
        std::vector<Connection*> connections;
        std::mutex connection_mutex;

    public:

    Scheduler() {
        connections.reserve(100);
    };

    Connection* AddNewRequest(RequestInfo request_info, httplib::ws::WebSocket& ws);

    void Run();


    void PRQ();

    void LST();
};