#ifndef DISPLAYHANDLER_H
#define DISPLAYHANDLER_H

#pragma once
#include "freertos/FreeRTOS.h"      // FreeRTOS
#include "freertos/semphr.h"        // FreeRTOS semaphore
#include "pin_config.h"
#include "display_config.h"
#include "drivers/GC9A01.h"         // LCD Driver

#include "lv_conf.h"                // LVGL Config
#include <lvgl.h>                   // LVGL Library
#include "lvgl/dashboard.h"         // LVGL Screen

class DisplayHandler {
private:
    GC9A01_Driver lcd;
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

    void TestGUI(void);

public:
    DisplayHandler();
    ~DisplayHandler();

    // Setup Task - called from the main program
    void RunTask(void);
};

#endif