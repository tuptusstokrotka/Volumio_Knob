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

#include "dev_tools.h"
#include <Preferences.h>
#include "wifi_config.h"
#include "webserver/webserver.h"
#include "volumio/volumio.h"
#include "../notify/NotificationManager.h"
#include "../notify/CommandQueue.h"
#include "../notify/TrackDataQueue.h"

#define RECONNECT_INTERVAL pdMS_TO_TICKS(5000)  // 5 seconds

class WiFiHandler {
private:
    AsyncWebServer* server  = nullptr;
    DNSServer* dns          = nullptr;

    // Config
    String ssid             = STA_SSID;     // Use arduino's String because of preferences
    String password         = STA_PASS;
    String volumioIP        = VOLUMIO_IP;

    // WiFi
    WiFiMode_t mode         = WIFI_STA;
    bool connected          = false;
    TickType_t lastAttempt  = 0;  // Track last reconnection attempt

    // Volumio
    Volumio* volumio        = nullptr;

    /**
     * @brief FreeRTOS task entry point
     * @param param pointer to the WiFiHandler instance
     */
    static void TaskEntry(void* param);

    void StartSTA(TickType_t timeout = 0);
    void StartAP(void);

    // Process Volumio commands queue
    void processVolumioCommands(void);

public:
    WiFiHandler();
    ~WiFiHandler();

    void RunTask(void);
    void Update();
    void ToggleMode();
};

#endif // WIFI_HANDLER_H