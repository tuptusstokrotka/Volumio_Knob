/**
 * @author Julian tuptusstokrotka@gmail.com
 */
#include "main.h"

void setup() {
    // ConfigManager::Initialize();

    display = new DisplayHandler();
    // board   = new BoardHandler(display);
    // ota     = new OtaHandler();

    display->RunTask();
    // board->RunTask();
}

void loop() {
    // Not used cause of FreeRTOS
}