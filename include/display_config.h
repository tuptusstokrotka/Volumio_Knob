#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#include "pins_arduino.h"

// GC9A01 240x240 Round Display Configuration
#define LCD_WIDTH       240
#define LCD_HEIGHT      240

// Pin Definitions
#define LCD_MOSI_PIN    G5
#define LCD_MISO_PIN    -1
#define LCD_SCLK_PIN    G6
#define LCD_DC_PIN      G4
#define LCD_CS_PIN      G7
#define LCD_RST_PIN     G8
#define LCD_BUSY_PIN    -1
#define LCD_BL_PIN      G9

// SPI Configuration
#define LCD_SPI_HOST    SPI3_HOST
#define LCD_SPI_MODE    0
#define LCD_FREQ_WRITE  80000000  // 80MHz
#define LCD_FREQ_READ   16000000  // 16MHz

// Backlight Configuration
#define LCD_BL_FREQ     44100
#define LCD_BL_CHANNEL  7



#define TOUCH_ENABLE    1

// Touch Configuration
#define TOUCH_IRQ       G14
#define TOUCH_RST       -1
#define TOUCH_I2C_PORT  I2C_NUM_0
#define TOUCH_I2C_ADDR  0x38
#define TOUCH_SDA       G11
#define TOUCH_SCL       G12

// LVGL Configuration
#define DRAW_BUF_SIZE   (LCD_WIDTH * LCD_HEIGHT * 2)  // *2 for switching between buffers
#define BUF_DIVIDER     4 // BUFFER DIVIDER for lvgl, partial rendering
// BUFFER = DRAW_BUF_SIZE / BUF_DIVIDER

#define TFT_ROTATION    LV_DISPLAY_ROTATION_0
#define DISPLAY_FPS     200      // Display task tick rate
#define SPLASH_SCREEN_TIME 1000 // Splash screen time

#endif // DISPLAY_CONFIG_H