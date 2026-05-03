#pragma once

#include <string>
#include "scheduler.h"

enum class MSG_STATE : uint8_t { 
    ERROR,
    NONE,
    TIMEOUT,
    SUCCESS
};

MSG_STATE VerifyHeader(std::string& request);


void NewRequest(const httplib::Request &req, httplib::ws::WebSocket &ws, Scheduler& scheduler);

void RequestHandler(const httplib::Request &req, httplib::ws::WebSocket &ws, Scheduler& scheduler);

void simpleRedirect(const httplib::Request &req, httplib::ws::WebSocket &ws, Scheduler& scheduler);