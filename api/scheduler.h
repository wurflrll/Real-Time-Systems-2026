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

    std::atomic<bool> finished{false};
    uint32_t id;

    Connection() { }
    Connection(RequestInfo req, httplib::ws::WebSocket& ws) { 
        id = rand() % 100000;
        request_info = req;
        socket = &ws;
        finished = false;
    }
    void InitialWork();
    bool Finished();
};


class Scheduler {

    private:
        std::vector<Connection*> connections;
        std::mutex connection_mutex;

    public:

    Scheduler() = default;

    Connection* AddNewRequest(RequestInfo request_info, httplib::ws::WebSocket& ws);

    void Run();
};