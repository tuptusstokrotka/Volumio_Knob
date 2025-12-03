#include "DisplayHandler.h"


void DisplayHandler::TestGUI(void){
    static int arcValue = 10; // Start value
    static int maxValue = 20; // Max value

    // Timer
    static TickType_t last_tick = xTaskGetTickCount();
    TickType_t now = xTaskGetTickCount();
    if(now - last_tick >= 1000 && now > 2000){
        last_tick = now;
        ++arcValue %= maxValue;
    }

    dashboard->SetArcValue(arcValue, maxValue);
    dashboard->SetTrackSeek(arcValue, maxValue);

    if(arcValue < maxValue / 4)
        dashboard->SetAccentColor(SPOTIFY_GREEN);
    else if(arcValue < maxValue / 2)
        dashboard->SetAccentColor(YOUTUBE_RED);
    else if(arcValue < maxValue * 3 / 4)
        dashboard->SetAccentColor(TIDAL_BLUE);
    else
        dashboard->SetAccentColor(ACCENT_COLOR);

    dashboard->SetShuffleIconState(arcValue < maxValue/2);
    dashboard->SetRepeatIconState(arcValue > maxValue/2);

    dashboard->SetStatus(arcValue < maxValue/3, arcValue < maxValue/3*2, arcValue < maxValue/3*3);
    dashboard->SetBatteryValue(map(arcValue, 0, maxValue, 0, 100));

    if(arcValue == 5)
        ShowPopup("Popup 3s", "This is a popup that will show for 3 seconds", 3000);
    if(arcValue == 12)
        ShowPopup("Popup Hard", "This is a popup that will stay until dismissed");
    if(arcValue == maxValue-1)
        HidePopup();
}

void DisplayHandler::TestPopup(void){
    const TickType_t popupAnimationDuration = pdMS_TO_TICKS(POPUP_ANIMATION_DURATION);
    const TickType_t popupShortDuration     = pdMS_TO_TICKS(1000 + 2*popupAnimationDuration);
    const TickType_t popupLongDuration      = pdMS_TO_TICKS(3000 + 2*popupAnimationDuration); // Long duration to test setting new timer
    const TickType_t popupInterruptDuration = pdMS_TO_TICKS(100  + 2*popupAnimationDuration); // Really short popup just to test animation interrupts

    static int testCase = 0;
    static int step = 0;

    static TickType_t lastActionTick = 0;
    const TickType_t now = xTaskGetTickCount();

    switch(testCase) {
        case 0: {
            // Case 1: No active popup - should play start animation
            if (step == 0) {
                HidePopup();
                lastActionTick = now;
                step = 1;
            }
            else if (step == 1 && (now - lastActionTick) >= popupAnimationDuration ) {
                ShowPopup("Case 0", "No active popup", popupShortDuration);
                lastActionTick = now;
                step = 2;
            }
            else if (step == 2 && (now - lastActionTick) >= popupShortDuration + popupAnimationDuration + pdMS_TO_TICKS(1000) ) {
                testCase = 1;
                step = 0;
            }
            break;
        }
        case 1: {
            // Case 2: Interrupted but NOT animating - should update content and restart timer
            if (step == 0) {
                ShowPopup("Case 1a", "Content", popupShortDuration);
                lastActionTick = now;
                step = 1;
            }
            else if (step == 1 && (now - lastActionTick) >= (popupAnimationDuration + popupShortDuration / 2) ) {
                ShowPopup("Case 1b", "Content\nUpdated", popupLongDuration);
                lastActionTick = now;
                step = 2;
            }
            else if (step == 2 && (now - lastActionTick) >= popupLongDuration + popupAnimationDuration + pdMS_TO_TICKS(1000) ) {
                testCase = 2;
                step = 0;
            }
            break;
        }
        case 2: {
            // Case 3: Interrupted during OPENING / CLOSING animation - should cancel and start fresh
            if (step == 0) {
                ShowPopup("Case 2a", "Opening", popupShortDuration);
                lastActionTick = now;
                step = 1;
            }
            else if (step == 1 && (now - lastActionTick) >= (popupAnimationDuration * 2 / 3) ) {
                ShowPopup("Case 2b", "Interrupted", popupLongDuration);
                lastActionTick = now;
                step = 2;
            }
            else if (step == 2 && (now - lastActionTick) >= (popupLongDuration + popupAnimationDuration / 2) ) {
                ShowPopup("Case 2c", "Closing", popupShortDuration);
                lastActionTick = now;
                step = 3;
            }
            else if (step == 3 && (now - lastActionTick) >= popupShortDuration + popupAnimationDuration + pdMS_TO_TICKS(1000) ) {
                testCase = 3;
                step = 0;
            }
            break;
        }
        case 3: {
            static TickType_t delay = popupAnimationDuration / 10;
            // Case 3: Interrupting during OPENING 10 times
            if(step < 10 && (now - lastActionTick) >= delay ){
                ShowPopup("Case 3", ("Opening\n" + std::to_string(delay) + "ms").c_str(), popupInterruptDuration);
                lastActionTick = now;
                delay += popupAnimationDuration / 10;
                step++;
            }

            if(step == 10 && (now - lastActionTick) >= popupInterruptDuration + popupAnimationDuration + pdMS_TO_TICKS(1000) ){
                testCase = 4;
                step = 0;
                delay = popupAnimationDuration / 10;
            }
            break;
        }
        case 4: {
            static TickType_t delay = popupInterruptDuration;
            // Case 4: Interrupting during CLOSING 10 times
            if(step < 10 && (now - lastActionTick) >= delay ){
                ShowPopup("Case 4", ("Closing\n" + std::to_string(delay-popupInterruptDuration) + "ms").c_str(), popupInterruptDuration);
                lastActionTick = now;
                delay += popupAnimationDuration / 10;
                step++;
            }
            if(step == 10 && (now - lastActionTick) >= popupInterruptDuration + popupAnimationDuration + pdMS_TO_TICKS(1000) ){
                testCase = 0;
                step = 0;
                delay = popupInterruptDuration;
            }
            break;
        }
    }
}