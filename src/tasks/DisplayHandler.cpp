#include "DisplayHandler.h"
#include "dev_tools.h"
#include "../lvgl/styles/styles.h"
#include "../notify/NotificationManager.h"

DisplayHandler::DisplayHandler(){
    // Display
    lcd.init();
    lcd.setRotation(TFT_ROTATION);
    touch.init();
    vTaskDelay(pdMS_TO_TICKS(10));

    // Encoder
    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);

    // LVGL setup
    draw_buf = (lv_color_t*)malloc(DRAW_BUF_SIZE/BUF_DIVIDER);
    if(draw_buf == nullptr) {
        return;
    }
    lv_init();
    lv_tick_set_cb(DisplayHandler::my_tick);

    // Setup LVGL display
    lv_display_t *lvDisplay = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    lv_display_set_color_format(lvDisplay, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(lvDisplay, DisplayHandler::DisplayFlush);
    lv_display_set_user_data(lvDisplay, this);
    lv_display_set_rotation(lvDisplay, TFT_ROTATION);
    lv_display_set_buffers(lvDisplay, draw_buf, NULL, DRAW_BUF_SIZE/BUF_DIVIDER, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Setup LVGL touch
    lv_indev_t* touch_indev = lv_indev_create();
    lv_indev_set_type(touch_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touch_indev, TouchEvent);
    lv_indev_set_user_data(touch_indev, this);

    // Setup LVGL encoder
    lv_indev_t* encoder_indev = lv_indev_create();
    lv_indev_set_type(encoder_indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(encoder_indev, EncoderEvent);
    lv_indev_set_user_data(encoder_indev, this);

    semaphore = xSemaphoreCreateMutex();

    // Subscribe to notifications
    NotificationManager::getInstance().subscribe(
        [this](const NotificationEvent& event) {
            this->handleNotification(event);
        }
    );
}

void DisplayHandler::TouchEvent(lv_indev_t *indev, lv_indev_data_t *data) {
    DisplayHandler* instance = static_cast<DisplayHandler*>(lv_indev_get_user_data(indev));
    if (instance == nullptr) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }

    instance->touch.readPos();
    FT3267::TouchPoint_t point = instance->touch.getTouchPointBuffer();

    #if TOUCH_VERBOSE == true
    DEBUG_PRINTLN("[Touch] Touch num: " << point.touch_num << ", X: " << point.x << ", Y: " << point.y);
    #endif

    if (point.touch_num > 0 && point.x >= 0 && point.y >= 0) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = point.x;
        data->point.y = point.y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void DisplayHandler::EncoderEvent(lv_indev_t *indev, lv_indev_data_t *data) {
    DisplayHandler* instance = static_cast<DisplayHandler*>(lv_indev_get_user_data(indev));
    if (instance == nullptr) {
        data->enc_diff = 0;
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }

    int32_t diff = instance->encoder.getDiff();
    data->enc_diff = diff;

    if (diff != 0 && instance->dashboard != nullptr) {
        instance->dashboard->OnVolumeChange(diff);
    }

    #if ENCODER_VERBOSE == true
    static int32_t last_logged_diff = 0;
    if (diff != 0 && diff != last_logged_diff) {
        DEBUG_PRINTLN("[Encoder] Diff: " << diff << ", Position: " << instance->encoder.getPosition());
        last_logged_diff = diff;
    }
    #endif

    if (gpio_get_level(BUTTON_PIN) == 0) {
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
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
                            3,              // Task priority
                            NULL,           // Task handle
                            0);             // Core ID (Core 0 - Fast Core)
}

void DisplayHandler::TaskEntry(void* param) {
    DisplayHandler* instance = static_cast<DisplayHandler*>(param);

    instance->dashboard = new Dashboard();
    lv_scr_load(instance->dashboard->GetScreen());

    while (true) {
        // instance->TestGUI();
        // instance->TestPopup();

        if(xSemaphoreTake(instance->semaphore, 10) == pdTRUE){
            lv_timer_handler();
            NotificationManager::getInstance().processNotifications();
            xSemaphoreGive(instance->semaphore);
        }
        vTaskDelay(1000 / DISPLAY_FPS);

    }
}

void DisplayHandler::ShowPopup(const char *title, const char *content, TickType_t duration) {
    if (dashboard != nullptr) {
        dashboard->ShowPopup(title, content, duration);
    }
}

void DisplayHandler::HidePopup(void) {
    if (dashboard != nullptr) {
        dashboard->HidePopup();
    }
}

void DisplayHandler::handleNotification(const NotificationEvent& event) {
    ShowPopup(event.title.c_str(), event.content.c_str(), pdMS_TO_TICKS(event.duration_ms));
}