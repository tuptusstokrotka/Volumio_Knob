#include "BoardHandler.h"
#include "dev_tools.h"
#include "../lvgl/styles/styles.h"
#include "../notify/NotificationManager.h"

BoardHandler::BoardHandler(){
    // Display
    lcd.init();
    lcd.setRotation(TFT_ROTATION);
    touch.init();
    vTaskDelay(pdMS_TO_TICKS(10));

    // Encoder
    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);

    // Battery
    Wire1.begin(SDA, SCL);
    vTaskDelay(pdMS_TO_TICKS(10));
    lipo = new Adafruit_MAX17048();
    if(!lipo->begin(&Wire1)) {
        delete lipo;
        lipo = nullptr;
        DEBUG_PRINTLN("[Battery] Failed to initialize MAX17048");
    }

    // LVGL setup
    draw_buf = (lv_color_t*)malloc(DRAW_BUF_SIZE/BUF_DIVIDER);
    if(draw_buf == nullptr) {
        return;
    }
    lv_init();
    lv_tick_set_cb(BoardHandler::my_tick);

    // Setup LVGL display
    lv_display_t *lvDisplay = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    lv_display_set_color_format(lvDisplay, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(lvDisplay, BoardHandler::DisplayFlush);
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

    // Setup LVGL battery
    lv_indev_t* battery_indev = lv_indev_create();
    lv_indev_set_type(battery_indev, LV_INDEV_TYPE_NONE);
    lv_indev_set_read_cb(battery_indev, BatteryEvent);
    lv_indev_set_user_data(battery_indev, this);

    semaphore = xSemaphoreCreateMutex();

    // Subscribe to notifications
    NotificationManager::getInstance().subscribe(
        [this](const NotificationEvent& event) {
            this->handleNotification(event);
        }
    );
}

void BoardHandler::TouchEvent(lv_indev_t *indev, lv_indev_data_t *data) {
    BoardHandler* instance = static_cast<BoardHandler*>(lv_indev_get_user_data(indev));
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

void BoardHandler::EncoderEvent(lv_indev_t *indev, lv_indev_data_t *data) {
    BoardHandler* instance = static_cast<BoardHandler*>(lv_indev_get_user_data(indev));
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

void BoardHandler::BatteryEvent(lv_indev_t *indev, lv_indev_data_t *data) {
    BoardHandler* instance = static_cast<BoardHandler*>(lv_indev_get_user_data(indev));
    if(instance == nullptr) {
        return;
    }

    if(instance->lipo == nullptr) {
        instance->dashboard->HideBatteryIcon();
        return;
    }

    instance->dashboard->SetBatteryValue((int)instance->lipo->cellPercent());
}

BoardHandler::~BoardHandler(){
    if (draw_buf != nullptr) {
        free(draw_buf);
        draw_buf = nullptr;
    }
    vSemaphoreDelete(semaphore);
    vTaskDelete(NULL);
}

uint32_t BoardHandler::my_tick(void) {
    return xTaskGetTickCount();
}

void BoardHandler::DisplayFlush(lv_display_t *display, const lv_area_t *area, unsigned char *data) {
    BoardHandler* instance = static_cast<BoardHandler*>(lv_display_get_user_data(display));
    if (instance == nullptr) return;

    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    lv_draw_sw_rgb565_swap(data, w * h);

    instance->lcd.pushImageDMA(area->x1, area->y1, w, h, (uint16_t *)data);

    lv_display_flush_ready(display); /* tell lvgl that flushing is done */
}

void BoardHandler::RunTask(void){
    xTaskCreatePinnedToCore(TaskEntry,      // Task entry point
                            "DisplayTask",  // Task name
                            8192,           // Stack depth
                            this,           // Task parameters - pointer to this instance
                            3,              // Task priority
                            NULL,           // Task handle
                            0);             // Core ID (Core 0 - Fast Core)
}

void BoardHandler::TaskEntry(void* param) {
    BoardHandler* instance = static_cast<BoardHandler*>(param);

    instance->dashboard = new Dashboard();
    lv_scr_load(instance->dashboard->GetScreen());

    while (true) {
        // instance->TestGUI();
        // instance->TestPopup();

        if(xSemaphoreTake(instance->semaphore, 10) == pdTRUE){
            lv_timer_handler();
            NotificationManager::getInstance().processNotifications();
            instance->processTrackData();
            xSemaphoreGive(instance->semaphore);
        }
        vTaskDelay(1000 / DISPLAY_FPS);
    }
}

void BoardHandler::ShowPopup(const char *title, const char *content, TickType_t duration) {
    if (dashboard != nullptr) {
        dashboard->ShowPopup(title, content, duration);
    }
}

void BoardHandler::HidePopup(void) {
    if (dashboard != nullptr) {
        dashboard->HidePopup();
    }
}

void BoardHandler::handleNotification(const NotificationEvent& event) {
    ShowPopup(event.title.c_str(), event.content.c_str(), pdMS_TO_TICKS(event.duration_ms));
}

void BoardHandler::processTrackData(void) {
    if (dashboard == nullptr) {
        return;
    }

    static TickType_t last_update_time = 0;
    TickType_t now = xTaskGetTickCount();

    if (now - last_update_time < pdMS_TO_TICKS(500)) {
        return;
    }
    last_update_time = now;

    Info trackData;
    std::string tempString;
    while (TrackDataQueue::getInstance().getTrackData(trackData)) {
        tempString = (trackData.title == "null" ? "-" : trackData.title);
        dashboard->SetTrackTitle(tempString.c_str());

        if(trackData.artist == "null" && trackData.album == "null")
            tempString = "-";
        else
            tempString = (trackData.artist == "null" ? "-" : trackData.artist) + " - " + (trackData.album == "null" ? "-" : trackData.album);
        dashboard->SetTrackArtist(tempString.c_str());

        tempString = (trackData.samplerate == "null" ? "-" : trackData.samplerate) + " / " + (trackData.bitdepth == "null" ? "-" : trackData.bitdepth);
        dashboard->SetTrackSamplerate(tempString.c_str());

        dashboard->SetTrackSeek(trackData.seek / 1000, trackData.duration);

        dashboard->SetStatus(trackData.status == "play");

        dashboard->SetRepeatIconState(trackData.repeat, trackData.repeatSingle);
        dashboard->SetRandomIconState(trackData.random);

        if(trackData.trackType == "spotify"){
            dashboard->SetPlayerIcon("spotify");
            dashboard->SetAccentColor(SPOTIFY_GREEN);
        }
        else if(trackData.trackType == "youtube"){
            dashboard->SetPlayerIcon("youtube");
            dashboard->SetAccentColor(YOUTUBE_RED);
        }
        else if(trackData.trackType == "tidal"){
            dashboard->SetPlayerIcon("tidal");
            dashboard->SetAccentColor(TIDAL_BLUE);
        }
        else if(trackData.trackType == "airplay"){
            dashboard->SetPlayerIcon("airplay");
            dashboard->SetAccentColor(AIRPLAY_COLOR);
        }
        else{
            dashboard->SetPlayerIcon(LV_SYMBOL_AUDIO);
            dashboard->SetAccentColor(ACCENT_COLOR);
        }
    }
}