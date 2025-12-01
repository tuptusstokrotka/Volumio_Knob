#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#pragma once

#include <WiFi.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <functional>

#include "wifi_config.h"
#include "volumio/volumio.h"
#include "webserver/webserver.h"


class WiFiHandler {
private:
    AsyncWebServer* server  = nullptr;
    DNSServer* dns          = nullptr;

    WiFiMode_t mode         = WIFI_STA;
    bool connected          = false;

    std::string ssid        = "";
    std::string password    = "";

public:
    WiFiHandler(std::string ssid, std::string password);
    ~WiFiHandler();

    bool isConnected() const { return connected; }
    void SwitchMode(WiFiMode_t mode);

    void setOnStart(std::function<void()> callback);
    void setOnProgress(std::function<void(size_t, size_t)> callback);
    void setOnEnd(std::function<void(bool)> callback);
};

#endif // WIFI_HANDLER_H