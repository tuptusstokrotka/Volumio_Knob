#pragma once

#include "freertos/FreeRTOS.h"

// Tasks
#include "tasks/DisplayHandler.h"
#include "tasks/WiFiHandler.h"

DisplayHandler* display = nullptr;
WiFiHandler* wifi = nullptr;