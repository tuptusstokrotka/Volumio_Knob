#ifndef DISPLAYHANDLER_H
#define DISPLAYHANDLER_H

#pragma once
#include "freertos/FreeRTOS.h"      // FreeRTOS
#include "freertos/semphr.h"        // FreeRTOS semaphore
#include "pin_config.h"
#include "display_config.h"
#include "drivers/GC9A01.h"         // LCD Driver
#include "drivers/FT3267.h"         // Touch Driver
#include "../board/Encoder.h"

#include "lv_conf.h"                // LVGL Config
#include <lvgl.h>                   // LVGL Library
#include "lvgl/dashboard.h"         // LVGL Screen

class DisplayHandler {
private:
    GC9A01_Driver lcd;
    FT3267::TP_FT3267 touch;
    Encoder encoder;

    SemaphoreHandle_t semaphore;

    lv_color_t* draw_buf = nullptr;
    Dashboard* dashboard = nullptr;

    static uint32_t my_tick(void);

    static void DisplayFlush(lv_display_t *display, const lv_area_t *area, unsigned char *data);

    /**
     * @brief FreeRTOS task entry point
     * @param param pointer to the display instance
     */
    static void TaskEntry(void* param);

    /**
     * @brief LVGL touch input device read callback
     */
    static void TouchEvent(lv_indev_t *indev, lv_indev_data_t *data);

    /**
     * @brief LVGL encoder input device read callback
     */
    static void EncoderEvent(lv_indev_t *indev, lv_indev_data_t *data);

    void TestGUI(void);

public:
    DisplayHandler();
    ~DisplayHandler();

    // Setup Task - called from the main program
    void RunTask(void);

    // Show popup notification
    void ShowPopup(const char *title, const char *content, TickType_t duration = 0);
    void HidePopup(void);
};

#endif