#include "WiFiHandler.h"

WiFiHandler::WiFiHandler(std::string ssid, std::string password) : ssid(ssid), password(password) {
    // Start always as STA
    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(500);
    }

    dns = new DNSServer();
    dns->start(53, "*", WiFi.localIP());

    if(!MDNS.begin(DNS_NAME))
        return;

    server = new AsyncWebServer(80);
    server->begin();

    #if DISABLE_OTA == true
        DEBUG_PRINTLN("[OTA] DISABLED");
        return;
    #elif defined(OTA_SSID) && defined(OTA_PASS)
        ElegantOTA.begin(server, OTA_SSID, OTA_PASS);
    #else
        ElegantOTA.begin(server);
    #endif

    webServerCallbacks(*server, WiFi.localIP());
}

WiFiHandler::~WiFiHandler() {
    if (server) {
        delete server;
        server = nullptr;
    }
}

// Set callbacks from main.cpp
void WiFiHandler::setOnStart(std::function<void()> callback) {
    ElegantOTA.onStart(callback);
}

void WiFiHandler::setOnProgress(std::function<void(size_t, size_t)> callback) {
    ElegantOTA.onProgress(callback);
}

void WiFiHandler::setOnEnd(std::function<void(bool)> callback) {
    ElegantOTA.onEnd(callback);
}


void WiFiHandler::SwitchMode(WiFiMode_t mode) {
    this->mode = mode;
    WiFi.disconnect();
    WiFi.mode(mode);
    if (mode == WIFI_STA) {
        WiFi.begin(ssid.c_str(), password.c_str());
    } else {
        WiFi.softAP(ssid.c_str(), password.c_str());
        dns->start(53, "*", WiFi.softAPIP());
    }
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(100);
    }
    this->connected = true;
}