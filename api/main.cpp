#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <ctime>
#include <chrono>
#include <string>
#include <iostream>

#include <vector>
#include <thread>

#include "cpp-httplib/httplib.h"
#include "scheduler.h"
#include "requests.h"

#define REQUEST_SIZE 12

using namespace std::literals::chrono_literals;

extern std::string terminal;


VideoFormat* video_format_1;


int main() {

    std::vector<Connection> connections;

    srand(time(NULL));

    httplib::Server svr;

    Scheduler scheduler;

    video_format_1 = new VideoFormat("media/video_1.mp4");


    svr.Options(R"(/.*)", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "http://davidrice.site");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_content("", "text/plain");
    });

    svr.Post("/start", [](const httplib::Request& req, httplib::Response& res) {
        
        auto name = req.get_param_value("name");

        std::cout << "THE NAME IS: " << name << "\n";

        name += std::to_string(rand() % 10000);

        res.set_header("Access-Control-Allow-Origin", "http://davidrice.site");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");

        res.set_content("Hello World! " + name, "text/plain");
    });


    svr.WebSocket("/ws", [&scheduler](const httplib::Request &req, httplib::ws::WebSocket &ws) {
       
        std::cout << "Inside of a new request\n";

        std::string message;

        Connection* new_connection = nullptr;

        bool found_terminator = false;

        while (ws.read(message)) {
            if (message.size() > REQUEST_SIZE) { 
                std::cout << "something screwed up, message too big\n";
                ws.send("Wrong termination... Message too big\n");
                return;
            }
            else if (message.size() < REQUEST_SIZE) {
                std::cout << "message is incomplete\n";
                return;
            }

            RequestInfo request_info;
            memcpy(&request_info, message.data(), REQUEST_SIZE);
            std::cout << "request info: " << request_info << "\n";
            new_connection = scheduler.AddNewRequest(request_info, ws);
            break;
        };
        auto start = std::chrono::high_resolution_clock::now();
        while (!new_connection->finished.load()) {   
            auto end = std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() > 200000) {
                std::cout << "a timeout has occurred\n";
                ws.send("timeout");
                break;
            }
            std::this_thread::sleep_for(50ms);
        }
        delete new_connection;
        ws.send("Message finished\n");


        std::cout << "MESSAGE OVER" << "\n\n";
        ws.close();

    });


    std::thread scheduler_thread(&Scheduler::Run, &scheduler);

    svr.listen("0.0.0.0", 8080);
}