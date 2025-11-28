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

class Volumio {
private:
    std::string *ip;
    std::string Response = std::string("");
    bool isConnected = false;
    bool isPlaying = false;

    inline bool VolumioStatus(void) { return isConnected; }
    inline bool CheckResponse(void) { return Response != std::string(""); }

public:
    Volumio(std::string& ip);
    ~Volumio();

    void Update(void);
    void ParseResponse(Info* trackdata);
    void SendCommand(std::string command);
};

#endif