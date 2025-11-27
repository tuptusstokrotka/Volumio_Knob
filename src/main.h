#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Developer Tools and Debugging
#include "dev_tools.h"

// Tasks
#include "tasks/DisplayHandler.h"
// #include "tasks/BoardHandler.h"
// #include "tasks/OtaHandler.h"

DisplayHandler* display = nullptr;
// BoardHandler*   board   = nullptr;
// OtaHandler*     ota     = nullptr;