#pragma once

#include <ESPAsyncWebServer.h>
#include <IPAddress.h>
#include "build/html.h"
// #include "ConfigManager.h"
// #include <ArduinoJson.h>


inline void webServerCallbacks(AsyncWebServer& server, const IPAddress& localIP){
    const String IP_URL = "http://" + localIP.toString();

    server.onNotFound(                  [IP_URL](AsyncWebServerRequest *request)    { request->redirect(IP_URL); });               // Not found redirect

    // Required for captive portal - this is stolen from the other project
    server.on("/connecttest.txt",       [](AsyncWebServerRequest *request)          { request->redirect("http://logout.net"); });  // Windows 11 captive portal workaround
    server.on("/wpad.dat",              [](AsyncWebServerRequest *request)          { request->send(404); });                      // Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)

    server.on("/generate_204",          [IP_URL](AsyncWebServerRequest *request)    { request->redirect(IP_URL); });               // Android captive portal redirect
    server.on("/redirect",              [IP_URL](AsyncWebServerRequest *request)    { request->redirect(IP_URL); });               // Microsoft redirect
    server.on("/hotspot-detect.html",   [IP_URL](AsyncWebServerRequest *request)    { request->redirect(IP_URL); });               // Apple call home
    server.on("/canonical.html",        [IP_URL](AsyncWebServerRequest *request)    { request->redirect(IP_URL); });               // Firefox captive portal call home
    server.on("/success.txt",           [](AsyncWebServerRequest *request)          { request->send(200); });                      // Firefox captive portal call home
    server.on("/ncsi.txt",              [IP_URL](AsyncWebServerRequest *request)    { request->redirect(IP_URL); });               // Windows call home

    server.on("/chrome-variations/seed",[](AsyncWebServerRequest *request)          { request->send(200); });                      // Chrome captive portal call home
    server.on("/service/update2/json",  [](AsyncWebServerRequest *request)          { request->send(200); });                      // Firefox?
    server.on("/chat",                  [](AsyncWebServerRequest *request)          { request->send(404); });                      // No stop asking WhatsApp, there is no internet connection
    server.on("/startpage",             [IP_URL](AsyncWebServerRequest *request)    { request->redirect(IP_URL); });

    // Serve the HTML file
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncWebServerResponse *response = request->beginResponse(200, "text/html", (const uint8_t*)html, html_len);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });

    // server->on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
    //     ConfigManager& config = ConfigManager::getInstance();

    //     DynamicJsonDocument doc(100);
    //     doc["ssid"] = config.getSsid();
    //     doc["ip"] = config.getIp();

    //     String response;
    //     serializeJson(doc, response);
    //     request->send(200, "application/json", response);
	// });

    // server->on("/post", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    //     // get posted data and update

    //     String body = "";
    //     for (size_t i = 0; i < len; i++) {
    //         body += (char)data[i];  // Convert incoming bytes to String
    //     }

    //     DynamicJsonDocument doc(256);
    //     DeserializationError error = deserializeJson(doc, body);

    //     if (error) {
    //         request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"Invalid JSON\"}");
    //         return;
    //     }

    //     String ssid     = doc["ssid"].as<String>();
    //     String password = doc["pass"].as<String>();
    //     String volumio  = doc["ip"].as<String>();

    //     /* Get and Save new config */
    //     ConfigManager& config = ConfigManager::getInstance();
    //     config.save(ssid, password, volumioIP);

    //     request->send(200, "application/json", "{\"status\":\"success\"}");
    // });
}