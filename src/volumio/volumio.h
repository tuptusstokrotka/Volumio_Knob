#ifndef VOLUMIO_H
#define VOLUMIO_H

#pragma once

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <string>
#include "volumio_trackdata.h"
#include "volumio_commands.h"
#include "dev_tools.h"

#if VOLUMIO_VERBOSE == true
    #define VOLUMIO_DEBUG_PRINT(s)   DEBUG_PRINT(s);
    #define VOLUMIO_DEBUG_PRINTLN(s) DEBUG_PRINTLN(s);
#else
    #define VOLUMIO_DEBUG_PRINT(s)   ;
    #define VOLUMIO_DEBUG_PRINTLN(s) ;
#endif

#define VOLUMIO_DEEP_SLEEP_INTERVAL pdMS_TO_TICKS( 5 * 60 * 1000)    // 5 minutes

class Volumio {
private:
    std::string ip;
    std::string Response = std::string("");
    bool connected = false;
    bool wasConnected = false;

    TickType_t disconnectTime = 0; // Track time of last disconnection

    inline bool CheckResponse(void) { return Response != std::string(""); }

public:
    Volumio(std::string ip);
    ~Volumio();

    inline bool isConnected(void) { return connected; }
    void SetIP(std::string ip) { this->ip = ip; }

    void Update(void);
    void ParseResponse(Info* trackdata);
    void SendCommand(std::string command);
};

#endif