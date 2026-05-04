#include "scheduler.h"

using namespace std::literals::chrono_literals;

uint32_t request_size = 12;


std::ostream& operator<<(std::ostream& os, const RequestInfo& req) {
    os << "start: " << req.start_second << "\n";
    os << "number frames: " << req.number_frames << "\n";
    os << "index: " << req.movie_index << "\n";
    return os;
}

void Connection::InitialWork() {
    char movie_file[50];
    switch(request_info.movie_index) {
        case 1:
            strcpy(movie_file, "movie_1.mp4"); break;
        case 2:
            strcpy(movie_file, "movie_2.mp4"); break;
        case 3:
            strcpy(movie_file, "movie_3.mp4"); break;
        default:
            strcpy(movie_file, "movie_3.mp4");    
    }

    // TODO: delete this: this is only for testing
    strcpy(movie_file, "media/video_1.mp4");  
                            
    if (!video_format.InitialRead(movie_file, request_info.start_second)) { 
        std::cout << "Initial Read Failed\n";
        finished = true;
        return;
    };

    // WANT TO CHOOSE TOTAL NUMBER OF FRAMES TO SEND

    //uint32_t total_frames = video_format.GetTotalFrames();

    uint32_t total_frames = request_info.number_frames;

    std::cout << "total frames: " << total_frames << "\n";

    video_buffer.frame_size = video_format.buffer_size;

    if (!video_format.ProcessFrames(total_frames, video_buffer, *socket)) { 
        std::cout << "frame processing failed\n";
        return;
    };
    //SendBuffer();

    ClearBuffer();

    finished = true;

    std::cout << "some initial work, id: " << id << "\n";
}

bool Connection::Finished() { 
    return finished;
}



void Connection::SendBuffer() {
    for (Frame& frame : video_buffer.frame_buffers) {
        //socket->send(reinterpret_cast<const char*>(frame_ptr), video_buffer.frame_size);
        std::cout << "FRAME SIZE: " << frame.size() << "\n";

        uint32_t half = frame.size() / 2;

        //socket->send(reinterpret_cast<const char*>(frame.data()), frame.size());
        socket->send(reinterpret_cast<const char*>(frame.data()), half);
        socket->send(reinterpret_cast<const char*>(frame.data() + half), frame.size() - half);

    }
}

void Connection::ClearBuffer() {
    // for (auto vec : video_buffer.frame_buffers) {
    //     free(vec);
    // }
}



Connection* Scheduler::AddNewRequest(RequestInfo request_info, httplib::ws::WebSocket& ws) {
    Connection* connection = new Connection(request_info, ws);
    connection_mutex.lock();
    connections.emplace_back(connection);
    connection_mutex.unlock();
    return connection;
}

void Scheduler::Run() {
    while (true) {
        std::cout << "running...\n";
        std::this_thread::sleep_for(2500ms);
        if (connections.size() == 0) {
            continue;
        }

        Connection* connection = connections[0];

        connection->InitialWork();
        connections.erase(connections.begin());
    }
}