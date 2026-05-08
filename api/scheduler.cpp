#include "scheduler.h"

using namespace std::literals::chrono_literals;

uint32_t request_size = 12;



extern VideoFormat* video_format_1;

int count_connections = 0;

std::ostream& operator<<(std::ostream& os, const RequestInfo& req) {
    os << "start: " << req.start_second << "\n";
    os << "number frames: " << req.number_frames << "\n";
    os << "index: " << req.movie_index << "\n";
    return os;
}

void Connection::InitialWork() {
    //char movie_file[50];
    // switch(request_info.movie_index) {
    //     case 1:
    //         strcpy(movie_file, "movie_1.mp4"); break;
    //     case 2:
    //         strcpy(movie_file, "movie_2.mp4"); break;
    //     case 3:
    //         strcpy(movie_file, "movie_3.mp4"); break;
    //     default:
    //         strcpy(movie_file, "movie_3.mp4");    
    // }
    /////TODO : REPLACE
    // .... video_format = video_format_n

    // TODO: delete this: this is only for experiment results
    //strcpy(movie_file, "media/video_1.mp4");  

    // if (!socket->is_open()) { 
    //     std::cout << "EARLY EXIT\n";
    //     return;
    // }

    video_format = video_format_1;

    frame_index = video_format->GetFrameIndex((double) request_info.start_second);

    std::cout << "FRAME INDEX: " << frame_index << "\n";

    video_format->ProcessFrames(frame_index, request_info.number_frames, *socket);

    count_connections++;

    std::cout << "Connections Count " << count_connections << "\n";

    finished.store(true);

    std::cout << "some initial work, id: " << id << "\n";
}

bool Connection::Finished() { 
    return finished;
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

        connection_mutex.lock();
        connections.erase(connections.begin());
        connection_mutex.unlock();
    }
}