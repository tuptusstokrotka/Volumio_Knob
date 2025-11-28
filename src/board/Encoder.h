#pragma once

#include "ESP32Encoder.h"
#include "../include/pin_config.h"
// #include "driver/gpio.h"

class Encoder {
private:
    ESP32Encoder encoder;
    int32_t last_position = 0;

public:
    Encoder(gpio_num_t pinA = ENCODER_A, gpio_num_t pinB = ENCODER_B);
    ~Encoder();

    int32_t getPosition();
    int32_t getDiff();
    void reset();
};

