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

    count_connections++;

    std::cout << "Connections Count " << count_connections << "\n";

    finished.store(true);

    start_time = std::chrono::high_resolution_clock::now();
    std::cout << "some initial work, id: " << id << "\n";
}

bool Connection::Finished() { 
    return finished;
}


// returns true if processed all frames
bool Connection::GetFrames(uint32_t num_frames) { 

    if (num_frames > request_info.number_frames) {
        num_frames = request_info.number_frames;
        request_info.number_frames = 0;
    }
    else {
        request_info.number_frames -= num_frames;
    }
    frame_index += num_frames;
    video_format->ProcessFrames(frame_index, num_frames, *socket);

    frames_sent += num_frames; 

    if (request_info.number_frames == 0) {
        return true;
    }
    return false;
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
        LST();
    }
}

void Scheduler::LST() {

    double frame_to_duration = 100000000;
    int index = -1;

    for (int i = 0; i < connections.size(); ++i) {
        Connection* connection = connections[i];
        if (!connection->initialized) {
            connection->InitialWork();
        }
        auto time_now = std::chrono::high_resolution_clock::now();
        uint32_t time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>
        (time_now - connection->start_time).count();

        double ratio = ((double) connection->frames_sent) / time_elapsed;
        if (ratio < frame_to_duration) {
            frame_to_duration = ratio;
            index = i;
        }
    }
    if (index != -1 && connections[index]->GetFrames(100)) {
        connection_mutex.lock();
        connections.erase(connections.begin() + index);
        conncetion_mutex.unlock();
    }
}