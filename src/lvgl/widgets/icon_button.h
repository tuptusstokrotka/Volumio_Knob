#pragma once

#include "lvgl.h"
#include "../styles/styles.h"

class IconButton {
private:
    lv_obj_t* button;
    lv_obj_t* label;

public:
    /**
     * @brief Create an icon button widget
     * @param parent Parent object (usually screen)
     * @param icon_text Icon symbol or text (e.g., LV_SYMBOL_PLAY)
     * @param size Button size (width and height)
     * @param font Font to use for the icon
     * @param color Text color
     * @param callback Event callback function
     * @param user_data User data to pass to callback
     */
    IconButton(lv_obj_t* parent) {
        // Button
        button = lv_btn_create(parent);
        lv_obj_set_style_bg_opa(button, LV_OPA_0, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(button, LV_OPA_40, LV_STATE_PRESSED); // Slight highlight when pressed

        lv_obj_set_style_border_width(button, 0, LV_PART_MAIN);
        lv_obj_set_style_border_width(button, 0, LV_STATE_PRESSED);

        lv_obj_set_style_outline_width(button, 0, LV_PART_MAIN);
        lv_obj_set_style_outline_width(button, 0, LV_STATE_PRESSED);

        lv_obj_set_style_shadow_width(button, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(button, 0, LV_STATE_PRESSED);

        SetButtonPressedColor(ACCENT_COLOR); // Default color

        // Label
        label = lv_label_create(button);
        lv_obj_center(label);
    }

    ~IconButton() {
        if (button != nullptr) {
            lv_obj_del(button);
            button = nullptr;
            label = nullptr;
        }
    }

    lv_obj_t* GetButton(void) { return button; }
    lv_obj_t* GetLabel(void) { return label; }

    void SetCallback(lv_event_cb_t callback, void* user_data = nullptr) {
        if (button != nullptr) {
            lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, user_data);
        }
    }

    void Align(lv_align_t align, int x_ofs = 0, int y_ofs = 0) {
        if (button != nullptr) {
            lv_obj_align(button, align, x_ofs, y_ofs);
        }
    }

    void SetButtonPressedColor(lv_color_t color) {
        if (button != nullptr) {
            lv_obj_set_style_bg_color(button, color, LV_STATE_PRESSED);
        }
    }

    void SetStyle(int size, const lv_font_t* font = SMALL_ICON_FONT) {
        if (button != nullptr) {
            lv_obj_set_size(button, size, size);
            lv_obj_set_style_radius(button, size / 2, LV_PART_MAIN);
            lv_obj_set_style_radius(button, size / 2, LV_STATE_PRESSED);
        }
        if (label != nullptr) {
            lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
        }
    }

    void SetIcon(const char* icon_text) {
        if (label != nullptr) {
            lv_label_set_text(label, icon_text);
        }
    }

    void SetIconColor(lv_color_t color) {
        if (label != nullptr) {
            lv_obj_set_style_text_color(label, color, LV_PART_MAIN);
        }
    }
};

