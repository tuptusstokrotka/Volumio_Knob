#include "Encoder.h"

Encoder::Encoder(gpio_num_t pinA, gpio_num_t pinB) : encoder() {
    gpio_set_pull_mode(pinA, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(pinB, GPIO_PULLUP_ONLY);

    encoder.attachHalfQuad(pinA, pinB);
    encoder.clearCount();
    last_position = 0;
}
Encoder::~Encoder() { }

int32_t Encoder::getPosition() {
    return encoder.getCount();
}

int32_t Encoder::getDiff() {
    int32_t current_position = encoder.getCount();
    int32_t diff = current_position - last_position;
    last_position = current_position;
    return diff;
}

void Encoder::reset() {
    encoder.clearCount();
    last_position = 0;
}
