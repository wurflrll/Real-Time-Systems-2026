#include "requests.h"

RequestInfo null_request = {};

extern uint32_t request_size;

#define REQUEST_SIZE 12

using namespace std::literals::chrono_literals;

std::string terminal = "Header end.";


void simpleRedirect(const httplib::Request &req, httplib::ws::WebSocket &ws, Scheduler& scheduler) {
    std::string msg;
    std::string whole_message;
    while (ws.read(msg)) {
        ws.send("echo: " + msg);
        whole_message += msg;
        Connection* new_connection = nullptr;
        std::cout << "Read a message here\n";


        std::cout << "Message size: " << msg.size() << "\n";
        for (char c : msg) {
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

    }
}

void NewRequest(const httplib::Request &req, httplib::ws::WebSocket &ws, Scheduler& scheduler) {

    std::cout << "Inside of a new request: "
    "MY NUMBER HERE"
    "\n";
    std::string whole_message;
    std::string message;

    Connection* new_connection = nullptr;

    bool found_terminator = false;


    std::cout << "original read value: " << ws.read(message) << "\n";

    whole_message += message;

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
        if (std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() > 20000) {
            std::cout << "a timeout has occurred\n";
            ws.send("timeout");
            break;
        }
        std::this_thread::sleep_for(50ms);
    }
    delete new_connection;

    std::cout << "Leaving the functions\n";
    ws.send("Message finished\n");
}

/*
MSG_STATE VerifyHeader(std::string& request) {

    if (request.size() > request_size + terminal.size()) { 
        return MSG_STATE::ERROR;
    }

    if (request.size() < request_size + terminal.size()) {
        return MSG_STATE::NONE;
    }

    assert(request.size() == terminal.size() + request_size);

    bool found_terminator = true;
    for (int i = request.size() - request.size(); i < request.size(); ++i) { 
        if (request[i] != terminal[i - (request.size() - terminal.size())]) { 
            found_terminator = false;
            break;
        }
    }
    if (!found_terminator) {
        std::cout << "Wrong termination... Right size\n";
        return MSG_STATE::ERROR;
    }
    
    return MSG_STATE::SUCCESS;
};

void RequestHandler(const httplib::Request &req, httplib::ws::WebSocket &ws, Scheduler& scheduler) {
    std::string message = {};
    std::string new_bytes;

    ws.send("opening - MY NUMBER HERE\n");

    std::cout << "Entered handler\n";

    std::cout << "ws.read(): " << (ws.read(new_bytes)) << "\n";

    while (ws.read(new_bytes)) {
        std::cout << "Entered while\n";
        ws.send("enters while\n");
        message += new_bytes;

        MSG_STATE status = VerifyHeader(message);

        if (status == MSG_STATE::NONE) { 
            std::cout << "In process of sending\n";
            continue;
        }
        else if (status == MSG_STATE::ERROR) {
            std::cout << "In correct header\n";
            ws.send("incorrect header\n");
            return;
        }
        assert(status == MSG_STATE::SUCCESS);

        RequestInfo request_info;
        memcpy(&request_info, message.data(), request_size);
        Connection* new_connection = scheduler.AddNewRequest(request_info, ws);

        ws.send("Creating a new connection\n");
        while (!new_connection->Finished()) { 
            
        }
        message = {};
    }

    std::cout << "somehow leaving\n";
    ws.send("leaving out\n");
}

*/