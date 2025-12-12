/**
 * @author Julian tuptusstokrotka@gmail.com
 */
#include "main.h"

void setup() {
    board = new BoardHandler();
    board->RunTask();

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    wifi = new WiFiHandler();
    wifi->RunTask();
}

void loop() {
    // Not used cause of FreeRTOS
}