#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Developer Tools and Debugging
#include "dev_tools.h"
#include "wifi_config.h"

// Tasks
#include "tasks/DisplayHandler.h"
#include "tasks/WiFiHandler.h"

// Config
#include "config/ConfigManager.h"

DisplayHandler* display = nullptr;
WiFiHandler* wifi = nullptr;