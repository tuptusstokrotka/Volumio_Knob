#pragma once

#include "freertos/FreeRTOS.h"

// Tasks
#include "tasks/BoardHandler.h"
#include "tasks/WiFiHandler.h"

BoardHandler* board = nullptr;
WiFiHandler* wifi = nullptr;