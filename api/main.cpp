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


int main() {

    std::vector<Connection> connections;

    srand(time(NULL));

    httplib::Server svr;

    Scheduler scheduler;


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

        std::string whole_message;
        std::string message;

        Connection* new_connection = nullptr;

        bool found_terminator = false;


        //std::cout << "original read value: " << ws.read(message) << "\n";

        //whole_message += message;

        while (ws.read(message)) {
            std::cout << "Read a message here\n";
            whole_message += message;
            std::cout << "Message size: " << message.size() << "\n";
            for (char c : message) {
                std::cout << c;
            }
            std::cout << "====end of string\n";

            std::cout << "whole message size: " << whole_message.size() << "\n";

            std::cout << "Request size: " << REQUEST_SIZE << "\n";

            std::cout << "terminal size: " << terminal.size() << "\n";

            if (whole_message.size() > REQUEST_SIZE + terminal.size()) { 
                std::cout << "something screwed up, message too big\n";
                ws.send("Wrong termination... Message too big\n");
                break;
            }
            else if (whole_message.size() < REQUEST_SIZE + terminal.size()) {
                std::cout << "message is incomplete\n";
                continue;
            }

            assert(whole_message.size() == terminal.size() + REQUEST_SIZE);

            found_terminator = true;
            for (int i = whole_message.size() - terminal.size(); i < whole_message.size(); ++i) { 
                if (whole_message[i] != terminal[i - (whole_message.size() - terminal.size())]) { 
                    found_terminator = false;
                    break;
                }
            }
            if (!found_terminator) {
                std::cout << "Wrong termination...\n";
                ws.send("Wrong termination... Right size\n");
                return;
            }
            RequestInfo request_info;
            memcpy(&request_info, whole_message.data(), REQUEST_SIZE);
            std::cout << "request info: " << request_info << "\n";
            new_connection = scheduler.AddNewRequest(request_info, ws);
            break;
        };

        std::cout << "WAS HERE TOO\n";

        if (!found_terminator) {
            ws.send("bad terminator");
            assert(new_connection == nullptr);
            return;
        }
        ws.send("good terminator...");

        whole_message = {};
        
        auto start = std::chrono::high_resolution_clock::now();
        while (!new_connection->finished) {   
            auto end = std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() > 200000) {
                std::cout << "a timeout has occurred\n";
                ws.send("timeout");
                break;
            }
            std::this_thread::sleep_for(50ms);
        }
        delete new_connection;

        std::cout << "Leaving the functions\n";

        //std::cout << "Leaving\n";
        ws.send("Message finished\n");
    });


    std::thread scheduler_thread(&Scheduler::Run, &scheduler);

    svr.listen("0.0.0.0", 8080);
}