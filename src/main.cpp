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
    display->RunTask();

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    wifi = new WiFiHandler(ssid, pass, display);
}

void loop() {
    // Not used cause of FreeRTOS
}