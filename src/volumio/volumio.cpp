#include "Volumio.h"
#include "../notify/NotificationManager.h"

Volumio::Volumio(std::string ip) : ip(ip) { }
Volumio::~Volumio(){ }

void Volumio::Update(void){
    if (WiFi.status() != WL_CONNECTED) {
        connected = false;
        return;
    }

    if (wasConnected != connected) {
        if (connected) {
            NotificationManager::getInstance().postNotification(
                "Volumio",
                "Connected",
                5000
            );
        } else {
            NotificationManager::getInstance().postNotification(
                "Volumio",
                "Connection lost",
                5000
            );
        }
    }
    wasConnected = connected;

    HTTPClient http;
    std::string volumioURL = "http://" + ip + ":3000/api/v1/getState";
    http.begin(volumioURL.c_str());
    int httpCode = http.GET();

    if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
        Response = std::string(http.getString().c_str());
        VOLUMIO_DEBUG_PRINTLN("[VOLUMIO] Update success");
        connected = true;
    }
    else{
        Response = std::string("");
        VOLUMIO_DEBUG_PRINTLN("[VOLUMIO] Update failed");
        connected = false;
    }
    http.end();
}

void Volumio::ParseResponse(Info *trackdata){
    JsonDocument doc;
    deserializeJson(doc, Response);

    trackdata->status        = doc["status"].as<std::string>();
    trackdata->title         = doc["title"].as<std::string>();
    trackdata->artist        = doc["artist"].as<std::string>();
    trackdata->album         = doc["album"].as<std::string>();
    trackdata->trackType     = doc["trackType"].as<std::string>();
    trackdata->seek          = doc["seek"];
    trackdata->duration      = doc["duration"];
    trackdata->samplerate    = doc["samplerate"].as<std::string>();
    trackdata->bitdepth      = doc["bitdepth"].as<std::string>();
    trackdata->random        = doc["random"].as<bool>();
    trackdata->repeat        = doc["repeat"].as<bool>();
    trackdata->repeatSingle  = doc["repeatSingle"].as<bool>();
}

void Volumio::SendCommand(std::string command){
    if (WiFi.status() != WL_CONNECTED || isConnected() == false)
        return;

    HTTPClient http;
    std::string volumioURL = "http://" + ip + "/api/v1/commands/?cmd=" + command;
    http.begin(volumioURL.c_str());
    int httpCode = http.GET();

    if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
        VOLUMIO_DEBUG_PRINTLN("[VOLUMIO] Command sent successfully: " << command);
    }
    else {
        VOLUMIO_DEBUG_PRINTLN("[VOLUMIO] Command failed: " << command);
    }
    http.end();
}