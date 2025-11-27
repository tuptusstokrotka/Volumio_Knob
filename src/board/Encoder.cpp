#include "Encoder.h"

Encoder::Encoder(int a, int b) : encoder(true, encoderISRCallback, this), last_value(0), current_value(0) {
    ESP32Encoder::useInternalWeakPullResistors = puType::down;

    // Attach encoder with interrupt enabled (full quadrature for all 32 changes per rotation)
    encoder.attachFullQuad(a, b);
    encoder.setCount(0);
    fast_step = true;

    // Create queue for encoder change notifications from ISR (just a trigger, no data needed)
    changeQueue = xQueueCreate(40, sizeof(uint8_t)); // Queue size 40, element size 1 byte (just notification)
    if (changeQueue == NULL) {
        #if ENCODER_VERBOSE == true
            DEBUG_PRINTLN("[ENCODER] Failed to create change queue");
        #endif
        return;
    }

    // Initialize cyclic buffer
    bufferIndex = 0;
    bufferFull = false;
    TickType_t now = xTaskGetTickCount();
    for (int i = 0; i < DYNAMIC_STEP_TRIG; i++) {
        changeTimes[i] = now;
    }
}

Encoder::~Encoder() {
    if (changeQueue != NULL) {
        vQueueDelete(changeQueue);
    }
}

void IRAM_ATTR Encoder::encoderISRCallback(void* arg) {
    Encoder* instance = static_cast<Encoder*>(arg);
    if (instance == NULL || instance->changeQueue == NULL) {
        return;
    }

    uint8_t trigger = 1;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(instance->changeQueue, &trigger, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

bool Encoder::getDirection(int64_t current_value) {
    bool direction = (current_value > this->last_value);
    if(invert)
        direction = !direction;
    return direction;
}

int64_t Encoder::getMultiplier() {
    int64_t stepMultiplier = SLOW_STEP;
    bool fastStepTriggered = false;

    /* Update cyclic buffer with current change time */
    TickType_t now = xTaskGetTickCount();
    changeTimes[bufferIndex] = now;

    /* Update index */
    bufferIndex = (bufferIndex + 1) % DYNAMIC_STEP_TRIG;
    if (bufferIndex == 0) {
        bufferFull = true;
    }

    if(fast_step) {
        if (bufferFull) {
            TickType_t oldestTime = changeTimes[bufferIndex]; // Next position is oldest
            TickType_t timeDiff = now - oldestTime;
            fastStepTriggered = (timeDiff <= DYNAMIC_STEP_WINDOW);
        }
        else {
            fastStepTriggered = (bufferIndex >= DYNAMIC_STEP_TRIG - 1);
        }
        stepMultiplier = fastStepTriggered ? FAST_STEP : SLOW_STEP;
    }
    return stepMultiplier;
}

void Encoder::update(void) {
    if (changeQueue == NULL) {
        return;
    }

    /* Check if there are any changes */
    uint8_t trigger;
    bool hasChanges = false;
    while (xQueueReceive(changeQueue, &trigger, 0) == pdTRUE) {
        hasChanges = true;
    }
    if (!hasChanges) {
        changed = false;
        return;
    }

    /* Get encoder reading */
    current_value = encoder.getCount();
    if(current_value == last_value){
        changed = false;
        return;
    }

    /* Calculate new position */
    bool direction = getDirection(current_value);
    int64_t stepMultiplier = getMultiplier();
    int64_t newPosition = last_value + stepMultiplier * (direction ? 1 : -1);
    newPosition = std::clamp(newPosition, min, max);

    /* Only update encoder if position changed */
    if (newPosition != current_value) {
        encoder.setCount(newPosition);
    }

    #if ENCODER_VERBOSE == true
        DEBUG_PRINTLN("[ENCODER] " << std::to_string(last_value) << (direction ? " + " : " - ") << std::to_string(stepMultiplier) << " -> " << std::to_string(newPosition) );
    #endif
    changed = newPosition != last_value;
    last_value = newPosition;
}

void Encoder::setAllValues(int64_t value, int64_t min, int64_t max, bool force) {
    setMin(min);
    setMax(max);

    // Must set the min/max first. Otherwise new value will be clamped to deprecated values
    setValue(value, force);
}

void Encoder::setValue(int64_t value, bool force) {
    value = std::clamp(value, min, max);

    encoder.setCount(value);
    current_value = value;
    last_value = value;

    force_changed = force;

    #if ENCODER_VERBOSE == true
        DEBUG_PRINTLN("[ENCODER] VALUE:" << std::to_string(value));
    #endif
}

void Encoder::setMin(int64_t value) {
    min = value;
    if(current_value < min){
        setValue(min, false);
    }

    #if ENCODER_VERBOSE == true
        DEBUG_PRINTLN("[ENCODER] MIN:" << std::to_string(min));
    #endif
}

void Encoder::setMax(int64_t value) {
    max = value;
    if(current_value > max){
        setValue(max, false);
    }

    #if ENCODER_VERBOSE == true
        DEBUG_PRINTLN("[ENCODER] MAX:" << std::to_string(max));
    #endif
}
