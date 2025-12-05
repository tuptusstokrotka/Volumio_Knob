#include "Volumio.h"
#include "../notify/NotificationManager.h"

Volumio::Volumio(std::string ip) : ip(ip) { }
Volumio::~Volumio(){ }

void Volumio::Update(void){
    // Post notifications on connection state changes
    if (!wasConnected && isConnected) {
        // Just connected
        NotificationManager::getInstance().postNotification(
            "Volumio",
            "Connected",
            5000
        );
    } else if (wasConnected && !isConnected) {
        // Connection lost
        NotificationManager::getInstance().postNotification(
            "Volumio",
            "Connection lost",
            5000
        );
    }

    wasConnected = isConnected;

    if (WiFi.status() != WL_CONNECTED) {
        isConnected = false;
        return;
    }

    HTTPClient http;
    std::string volumioURL = "http://" + ip + ":3000/api/v1/getState";
    http.begin(volumioURL.c_str());
    int httpCode = http.GET();

    if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
        Response = std::string(http.getString().c_str());
        VOLUMIO_DEBUG_PRINTLN("[VOLUMIO] Update success");
        isConnected = true;
    }
    else{
        Response = std::string("");
        VOLUMIO_DEBUG_PRINTLN("[VOLUMIO] Update failed");
        isConnected = false;
    }
    http.end();
}

void Volumio::ParseResponse(Info *trackdata){
    StaticJsonDocument<1024> doc;
    deserializeJson(doc, Response);

    trackdata->status        = doc["status"].as<std::string>();
    (trackdata->status == "play") ? isPlaying = true : isPlaying = false;

    trackdata->title         = doc["title"].as<std::string>();
    trackdata->artist        = doc["artist"].as<std::string>();
    trackdata->samplerate    = doc["samplerate"].as<std::string>();
    trackdata->bitdepth      = doc["bitdepth"].as<std::string>();
    trackdata->trackType     = doc["trackType"].as<std::string>();
    trackdata->duration      = doc["duration"];
    trackdata->seek          = doc["seek"];

    trackdata->elapsedTime   = trackdata->seek / 1000;
}

void Volumio::SendCommand(std::string command){
    if (WiFi.status() != WL_CONNECTED || VolumioStatus() == false)
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