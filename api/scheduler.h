#pragma once

#include <iostream>
#include <vector>
#include <mutex>
#include <chrono>
#include <thread>

#include "video.h"

#include "cpp-httplib/httplib.h"

#define REQUEST_SIZE 12


struct RequestInfo { 
    uint32_t start_second;
    uint32_t end_second;
    uint32_t movie_index;

    friend std::ostream& operator<<(std::ostream& os, const RequestInfo& req);

};


class Connection {

    private:
    
    RequestInfo request_info;
    VideoFormat video_format;
    VideoBuffer video_buffer;
    httplib::ws::WebSocket* socket;

    public:

    bool finished;
    uint32_t id;

    Connection() { }
    Connection(RequestInfo req, httplib::ws::WebSocket& ws) { 
        id = rand() % 100000;
        request_info = req;
        socket = &ws;
        finished = false;
    }
    void InitialWork();
    void SendBuffer();
    void ClearBuffer();
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