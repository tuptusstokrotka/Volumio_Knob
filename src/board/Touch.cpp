#include "Touch.h"

Touch::Touch(){
    #if TOUCH_ENABLE == 1
        init();
    #endif
}

Touch::~Touch(){ }

/**
 * @brief Get the touched state of the touch sensor
 * This must trigger one time per touch event
 * Ignoring hold and multiple touch events
 */
bool Touch::GetTouched(void){
    #if TOUCH_ENABLE == 0
        return false;
    #endif

    // Get touch
    bool touch = isTouched();
    // Get coordinates
    FT3267::TouchPoint_t point = getTouchPointBuffer();

    // Not touched
    if(touch == false){
        last_touched = false;
        return false;
    }

    // Touch held
    if(touch == true && last_touched == true){
        #if TOUCH_VERBOSE == true
            DEBUG_PRINTLN("[Touch] HELD - Ignoring");
        #endif
        return false;
    }

    // Incorrect Coords - ignore self triggered touch
    if(point.x == -1 && point.y == -1){
        #if TOUCH_VERBOSE == true
            DEBUG_PRINTLN("[Touch] OUT OF BOUNCE");
        #endif
        return false;
    }

    #if TOUCH_VERBOSE == true
        DEBUG_PRINTLN("[Touch] TOUCHED - [x,y] = [" << point.x << ", " << point.y << "]");
    #endif
    last_touched = touch;
    return true;
}