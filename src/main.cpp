/**
 * @author Julian tuptusstokrotka@gmail.com
 */
#include "main.h"

void setup() {
    std::string ssid = STA_SSID;
    std::string pass = STA_PASS;
    std::string ip = VOLUMIO_IP;

    ConfigManager::getInstance().begin(ssid, pass, ip);

    display = new DisplayHandler();
    wifi = new WiFiHandler(ssid, pass);

    display->RunTask();
}

void loop() {
    // Not used cause of FreeRTOS
}