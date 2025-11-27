#ifndef ENCODER_H
#define ENCODER_H

#pragma once
#include <ESP32Encoder.h>
#include <string>
#include <algorithm>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "pin_config.h"
#include "dev_tools.h"

#define DYNAMIC_STEP_WINDOW 200 // [ms]    Fast change window time
#define DYNAMIC_STEP_TRIG     6 // [steps] Fast change trigger steps
#define SLOW_STEP             1
#define FAST_STEP            20

/**
 * @brief Enhanced encoder class with dynamic step detection and value clamping
 *
 * This class provides an improved interface over ESP32Encoder with:
 * - Dynamic step detection (fast/slow scrolling)
 * - Value clamping with min/max limits
 * - Direction inversion support
 * - Change detection and direction tracking
 */
class Encoder {
private:
    ESP32Encoder encoder;
    bool invert = false; // Invert encoder direction

    bool changed = false;
    bool force_changed = false;

    // Current and last value
    int64_t current_value = 0;
    int64_t last_value    = 0;

    int64_t min = -1000; // Default min
    int64_t max =  1000; // Default max

    // Fast change detection
    bool fast_step = false;
    TickType_t changeTimes[DYNAMIC_STEP_TRIG]; // Cyclic buffer for last change times
    uint8_t bufferIndex = 0; // Current position in cyclic buffer
    bool bufferFull = false; // Whether buffer is full

    // Interrupt-based update notification
    QueueHandle_t changeQueue; // Queue for encoder change notifications from ISR
    static void IRAM_ATTR encoderISRCallback(void* arg); // Static ISR callback

    /**
     * @brief Get the direction of the encoder. Includes invert.
     * @return `TRUE` for forward, `FALSE` for backward
     */
    bool getDirection(int64_t current_value);
    /**
     * @brief Get the multiplier of the encoder. Includes dynamic step trigger.
     * @return Step multiplier `SLOW_STEP` or `FAST_STEP`.
     */
    int64_t getMultiplier();
public:
    /**
     * @brief Constructor
     * @param a Encoder pin A (default: ENCODER_A)
     * @param b Encoder pin B (default: ENCODER_B)
     */
    Encoder(int a = ENCODER_A, int b = ENCODER_B);

    /**
     * @brief Destructor
     */
    ~Encoder();

    /**
     * @brief Update the encoder value and detect changes
     * @note This method should be called regularly from the main loop
     * It processes interrupt queue notifications and reads encoder count
     */
    void update(void);

    // Configuration methods
    /**
     * @brief Set encoder direction inversion
     * @param value true to invert direction, false for normal
     */
    void setInvert(bool value) { invert = value; }

    /**
     * @brief Set fast step mode
     * @param enable true to enable dynamic fast stepping, false for constant slow step
     */
    void setFastStep(bool enable) { fast_step = enable; }

    // Value management
    /**
     * @brief Set all encoder values
     * @param value The value to set (will be clamped to min/max range)
     * @param min The minimum
     * @param max The maximum
     * @param force Whether to force the change detection
     */
    void setAllValues(int64_t value, int64_t min, int64_t max, bool force = false);

    /**
     * @brief Set encoder value and trigger change detection
     * @param value The value to set (will be clamped to min/max range)
     * @param force Whether to force the change detection
     */
    void setValue(int64_t value, bool force = true);

    /**
     * @brief Get current encoder value
     * @return Current encoder value
     */
    int64_t getValue(void) const { return last_value; }

    /**
     * @brief Set minimum value limit
     * @param value Minimum allowed value
     */
    void setMin(int64_t value);

    /**
     * @brief Get minimum value limit
     * @return Current minimum value
     */
    int64_t getMin(void) const { return min; }

    /**
     * @brief Set maximum value limit
     * @param value Maximum allowed value
     */
    void setMax(int64_t value);

    /**
     * @brief Get maximum value limit
     * @return Current maximum value
     */
    int64_t getMax(void) const { return max; }

    /**
     * @brief Check if encoder value has changed since last snapshot
     * @return true if value has changed
     */
    bool hasChanged(void) const { return changed || force_changed; }
};

#endif // ENCODER_H
