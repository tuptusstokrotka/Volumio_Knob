#pragma once

#include <LovyanGFX.hpp>
#include "display_config.h"

class GC9A01_Driver : public lgfx::LGFX_Device {
private:
    lgfx::Panel_GC9A01 _panel;
    lgfx::Bus_SPI _bus;
    lgfx::Light_PWM _backlight;

    void configureSPI() {
        auto cfg = _bus.config();
        cfg.spi_host = LCD_SPI_HOST;
        cfg.spi_mode = LCD_SPI_MODE;
        cfg.freq_write = LCD_FREQ_WRITE;
        cfg.freq_read = LCD_FREQ_READ;
        cfg.spi_3wire = true;
        cfg.use_lock = true;
        cfg.dma_channel = SPI_DMA_CH_AUTO;
        cfg.pin_sclk = LCD_SCLK_PIN;
        cfg.pin_mosi = LCD_MOSI_PIN;
        cfg.pin_miso = LCD_MISO_PIN;
        cfg.pin_dc = LCD_DC_PIN;
        _bus.config(cfg);
    }

    void configurePanel() {
        auto cfg = _panel.config();
        cfg.pin_cs = LCD_CS_PIN;
        cfg.pin_rst = LCD_RST_PIN;
        cfg.pin_busy = LCD_BUSY_PIN;
        cfg.panel_width = LCD_WIDTH;
        cfg.panel_height = LCD_HEIGHT;
        cfg.offset_x = 0;
        cfg.offset_y = 0;
        cfg.offset_rotation = 0;
        cfg.dummy_read_pixel = 8;
        cfg.dummy_read_bits = 1;
        cfg.readable = true;
        cfg.invert = true;
        cfg.rgb_order = false;
        cfg.dlen_16bit = false;
        cfg.bus_shared = true;
        _panel.config(cfg);
    }

    void configureBacklight() {
        auto cfg = _backlight.config();
        cfg.pin_bl = LCD_BL_PIN;
        cfg.invert = false;
        cfg.freq = LCD_BL_FREQ;
        cfg.pwm_channel = LCD_BL_CHANNEL;
        _backlight.config(cfg);
    }

public:
    GC9A01_Driver() {
        configureSPI();
        configurePanel();
        configureBacklight();

        _panel.setBus(&_bus);
        _panel.setLight(&_backlight);
        setPanel(&_panel);
    }

    // High-performance methods
    inline void pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t* data) {
        lgfx::LGFX_Device::pushImageDMA(x, y, w, h, data);
    }
};