#include "DisplayHandler.h"
#include "esp_heap_caps.h"
#include "dev_tools.h"
#include "lvgl/styles/styles.h"

DisplayHandler::DisplayHandler(){
    lcd.init();
    lcd.setRotation(TFT_ROTATION);

    draw_buf = (lv_color_t*)malloc(DRAW_BUF_SIZE/BUF_DIVIDER);
    if(draw_buf == nullptr) {
        return;
    }

    // LVGL setup
    lv_init();
    lv_tick_set_cb(DisplayHandler::my_tick);

    lv_display_t *lvDisplay = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    lv_display_set_color_format(lvDisplay, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(lvDisplay, DisplayHandler::DisplayFlush);
    lv_display_set_user_data(lvDisplay, this);
    lv_display_set_rotation(lvDisplay, TFT_ROTATION);
    lv_display_set_buffers(lvDisplay, draw_buf, NULL, DRAW_BUF_SIZE/BUF_DIVIDER, LV_DISPLAY_RENDER_MODE_PARTIAL);

    semaphore = xSemaphoreCreateMutex();
}

DisplayHandler::~DisplayHandler(){
    if (draw_buf != nullptr) {
        free(draw_buf);
        draw_buf = nullptr;
    }
    vSemaphoreDelete(semaphore);
    vTaskDelete(NULL);
}

uint32_t DisplayHandler::my_tick(void) {
    return xTaskGetTickCount();
}

void DisplayHandler::DisplayFlush(lv_display_t *display, const lv_area_t *area, unsigned char *data) {
    // Get the instance from the display user data
    DisplayHandler* instance = static_cast<DisplayHandler*>(lv_display_get_user_data(display));
    if (instance == nullptr) return;

    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    lv_draw_sw_rgb565_swap(data, w * h);

    instance->lcd.pushImageDMA(area->x1, area->y1, w, h, (uint16_t *)data);

    lv_display_flush_ready(display); /* tell lvgl that flushing is done */
}

void DisplayHandler::RunTask(void){
    xTaskCreatePinnedToCore(TaskEntry,      // Task entry point
                            "DisplayTask",  // Task name
                            8192,           // Stack depth
                            this,           // Task parameters - pointer to this instance
                            3,              // Task priority (HIGH - UI responsiveness)
                            NULL,           // Task handle
                            0);             // Core ID (Core 0 - Fast Core)
}

void DisplayHandler::TaskEntry(void* param) {
    DisplayHandler* instance = static_cast<DisplayHandler*>(param);

    // Create dashboard screen
    instance->dashboard = new Dashboard();
    lv_scr_load(instance->dashboard->GetScreen());

    while (true) {
        instance->TestGUI();

        if(xSemaphoreTake(instance->semaphore, 10) == pdTRUE){
            lv_timer_handler();
            xSemaphoreGive(instance->semaphore);
        }
        vTaskDelay(1000 / DISPLAY_FPS);

    }
}

void DisplayHandler::TestGUI(void){
    static int arcValue = 10;
    static int maxValue = 20;
    static TickType_t last_tick = xTaskGetTickCount();
    TickType_t now = xTaskGetTickCount();

    if(now - last_tick >= 1000 && now > 2000){
        last_tick = now;

        dashboard->SetArcValue(arcValue, maxValue);
        dashboard->SetTrackSeek(arcValue, maxValue);

        // Change accent color based on arcValue range
        if(arcValue < maxValue / 4){
            dashboard->SetAccentColor(SPOTIFY_GREEN);
        } else if(arcValue < maxValue / 2){
            dashboard->SetAccentColor(YOUTUBE_RED);
        } else if(arcValue < maxValue * 3 / 4){
            dashboard->SetAccentColor(TIDAL_BLUE);
        } else {
            dashboard->SetAccentColor(ACCENT_COLOR);
        }

        dashboard->SetStatus(arcValue < maxValue/3, arcValue < maxValue/3*2, arcValue < maxValue/3*3);
        dashboard->SetBatteryValue(map(arcValue, 0, maxValue, 0, 100));

        if(arcValue == 5)
            dashboard->popup->ShowPopup("Popup 3s", "This is a popup that will show for 3 seconds", 3000);
        if(arcValue == 12)
            dashboard->popup->ShowPopup("Popup Hard", "This is a popup that will stay until dismissed");
        if(arcValue == maxValue-1)
            dashboard->popup->HidePopup();

        ++arcValue %= maxValue;
    }
}