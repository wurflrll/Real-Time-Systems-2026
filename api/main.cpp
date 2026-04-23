#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "cpp-httplib/httplib.h"

#include <ctime>
#include <string>
#include <iostream>

#include <vector>
#include <thread>

#include "scheduler.cpp"

std::string terminal = "Header end.";

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

    svr.WebSocket("/ws", [](const httplib::Request &req, httplib::ws::WebSocket &ws) {
        
        
        std::string whole_message;

        ws.on_message = [&](const std::string &message) {
            whole_message += message;


            if (whole_message.length() - terminal.size() > REQUEST_SIZE) { 
                std::cout << "something screwed up\n";
                return;
            }
            else if (whole_message.length - terminal.size() != REQUEST_SIZE) {
                return;
            }

            bool found = false;
            for (int i = whole_message.length() - terminal.size(); i < whole_message.length(); ++i) { 
                if (i < 0) { 
                    break;
                }
                found = true;
                bool found = true;
                for (int k = 0; k < terminal.length(); ++k) { 
                    if (whole_message[i + k] != terminal[k]) {
                        found = false;
                        break;
                    }
                }
            }
            if (!found) {
                return;
            }

            RequestInfo request_info;
            memcpy(&request_info, whole_message.data(), REQUEST_SIZE);


            std::cout << "request info: " << request_info << "\n";

            scheduler.AddNewRequest(request_info);

            // message contains the incoming bytes (as a string)
        
            // You can respond if you want
            ws.send("Got it!\n");
        };
        std::string msg;
        ws.send("End of Response!\n");
    });

    // svr.Get("/ws", [](const httplib::Request&, httplib::Response& res) {
    //     res.set_content("hit ws as http", "text/plain");
    // });
    std::thread scheduler_thread(&Scheduler::Run, &scheduler);

    svr.listen("0.0.0.0", 8080);
}