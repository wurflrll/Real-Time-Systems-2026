#include <vector>
#include <mutex>
#include <chrono>
#include <thread>

#include "video.cpp"



using namespace std::literals::chrono_literals;

typedef struct { 
    uint32_t start_second;
    uint32_t end_second;
    uint32_t movie_index;

    friend std::ostream& operator<<(std::ostream& os, const RequestInfo& req) {
        os << "start: " << req.start_second;
        os << "end: " << req.end_second;
        os << "index: " << req.movie_index;
        return os;
    }

} RequestInfo;





class Connection {

    private:
    uint32_t start_second;
    uint32_t end_second;

    uint32_t total_frames;  // calculated from encoding
                    // and start / end second

    VideoFormat video;

    public:
    bool read_file; // true if file / codec has been completely read
    
    Connection(RequestInfo request_info) {

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

        if (!video.InitialRead(movie_file)) { 
            std::cout << "Initial Read Failed\n";
        };

        total_frames = video.GetTotalFrames();

        video.ProcessFrames(total_frames);

        SendFrames();
    };

    void SendFrames() {}
};



class Scheduler {


    private:
        std::vector<Connection*> connections;
        std::mutex connection_mutex;


    public:

    Scheduler() = default;

    void AddNewRequest(RequestInfo request_info) {
        Connection* connection = new Connection(request_info);
        connection_mutex.lock();
        connections.emplace_back(connection);
        connection_mutex.unlock();
    }

    void Run() {
        while (true) {

            std::cout << "running...\n";
            std::this_thread::sleep_for(2500ms);
            if (connections.size() == 0) {
                continue;
            }

            Connection* connection = connections[0];

            connection->InitialRead();
            while (!connection->SendFrames(100)) {
        
            }
            delete connection;
            connections.erase(connections.begin());
        }
    }
};