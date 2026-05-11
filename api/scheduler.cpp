#include "scheduler.h"

using namespace std::literals::chrono_literals;

uint32_t request_size = 12;

uint32_t granularity_x = 100;


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

    std::cout << "FRAME INDEX ORIGINAL: " << frame_index << "\n";


    initialized = true;

    count_connections++;

    std::cout << "Connections Count " << count_connections << "\n";
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

    video_format->ProcessFrames(frame_index, num_frames, *socket);

    frame_index += num_frames;

    frames_sent += num_frames; 

    assert(frame_index < 2500);

    if (request_info.number_frames == 0) {
        return true;
    }
    return false;
}

Connection* Scheduler::AddNewRequest(RequestInfo request_info, httplib::ws::WebSocket& ws) {
    Connection* connection = new Connection(request_info, ws);
    connection_mutex.lock();
    connections.emplace_back(connection);
    std::cout << "ADD ----- num connections: " << connections.size() << "\n";
    connection_mutex.unlock();
    return connection;
}

void Scheduler::Run() {
    while (true) {
        // decides which scheduler is active
        //LST();
        PRQ();
    }
}


// implementation of FIFO scheduler
void Scheduler::PRQ() {
    for (int i = 0; i < connections.size(); ++i) {
        Connection* connection = connections[i];
        connection->InitialWork();
        connection->GetFrames(1000000000); // max number to send all frames at once
 
        connection_mutex.lock();
        connections.erase(connections.begin() + i);
        connection_mutex.unlock();
    }

}

//implementation of preempting scheduler called "LST"
void Scheduler::LST() {
    // finding minimum of sent frames to connection time ratio
    double frame_to_duration = 100000000;
    Connection* chosen_connection = nullptr;
    {
        std::lock_guard<std::mutex> lock(connection_mutex);
        for (auto connection : connections) {
            if (!connection->initialized) {
                connection->InitialWork();
            }
            auto time_now = std::chrono::high_resolution_clock::now();
            uint32_t time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - connection->start_time).count();
            double ratio = ((double) connection->frames_sent) / (time_elapsed + 1);
            if (ratio < frame_to_duration) {
                frame_to_duration = ratio;
                chosen_connection = connection;
            }
        }
    }

    if (chosen_connection != nullptr) { // will be nullptr iff. no connection in queue
\        // if connection timeout in callback = 200s pass, or all frames sent -> remove from queue
        if(chosen_connection->finished.load() || chosen_connection->GetFrames(granularity_x)) {
            chosen_connection->finished.store(true);
            std::lock_guard<std::mutex> lock(connection_mutex);
            auto it = std::find(connections.begin(), connections.end(), chosen_connection);
            std::cout << "REMOVE ----- num connections: " << connections.size() << "\n";

            if (it != connections.end()) {
                connections.erase(it);
            }
        }
    }
}