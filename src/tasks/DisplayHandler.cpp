#include "DisplayHandler.h"
#include "esp_heap_caps.h"
#include "dev_tools.h"
#include "lvgl/styles.h"

DisplayHandler::DisplayHandler(){
    lcd.init();
    lcd.setRotation(TFT_ROTATION);

    draw_buf = (lv_color_t*)malloc(DRAW_BUF_SIZE/BUF_DIVIDER);
    if(draw_buf == nullptr) {

        return;
    }

    lv_init();
    lv_tick_set_cb(DisplayHandler::my_tick);

    // LVGL setup - improved configuration
    lv_display_t *lvDisplay = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    lv_display_set_color_format(lvDisplay, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(lvDisplay, DisplayHandler::DisplayFlush);
    lv_display_set_user_data(lvDisplay, this);  // Pass this instance to the display

    // Set rotation to match hardware
    lv_display_set_rotation(lvDisplay, TFT_ROTATION);

    // Configure buffers - ensure proper alignment and size
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

    int arcValue = 0;
    TickType_t last_tick = xTaskGetTickCount();

    while (true) {
        TickType_t now = xTaskGetTickCount();
        TickType_t elapsed = now - last_tick;
        if(elapsed >= 1000){
            last_tick = now;
            instance->dashboard->SetArcValue(arcValue % 100);  // Arc range is 0-100
            instance->dashboard->SetTrackSeek(arcValue, 100);
            arcValue += 15;
            arcValue %= 100;

            // Change accent color based on arcValue range
            if(arcValue < 25){
                instance->dashboard->SetAccentColor(SPOTIFY_GREEN);
            } else if(arcValue < 50){
                instance->dashboard->SetAccentColor(YOUTUBE_RED);
            } else if(arcValue < 75){
                instance->dashboard->SetAccentColor(TIDAL_BLUE);
            } else {
                instance->dashboard->SetAccentColor(ACCENT_COLOR);
            }
        }

        if(xSemaphoreTake(instance->semaphore, 10) == pdTRUE){
            lv_timer_handler();
            xSemaphoreGive(instance->semaphore);
        }
        vTaskDelay(1000 / DISPLAY_FPS);

    }
}