#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "lvgl.h"
#include "../styles/styles.h"
#include <string>

class lvgl_popup {
private:
    std::string title       = "";
    std::string content     = "";

    bool is_visible         = false;
    bool timeout_enabled    = false;

    TickType_t timeout_ms   = 0;
    TickType_t show_time    = 0;
    lv_timer_t *timeout_timer = nullptr;

    lv_obj_t *popup         = nullptr;
    lv_obj_t *title_label   = nullptr;
    lv_obj_t *content_label = nullptr;

    // Animation callbacks
    static void anim_popup_cb(void * var, int32_t v) {
        lv_obj_t * popup = static_cast<lv_obj_t*>(var);
        lv_obj_set_width(popup, lv_map(v, 0, 100, 0, POPUP_WIDTH));
        lv_obj_set_height(popup, lv_map(v, 0, 100, 0, POPUP_HEIGHT));
        lv_obj_set_style_text_opa(popup, lv_map(v, 0, 100, 0, POPUP_OPACITY), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(popup, lv_map(v, 0, 100, 0, POPUP_OPACITY), LV_PART_MAIN);
    }

    static void anim_completed_cb(lv_anim_t * a) {
        lv_obj_t * popup = static_cast<lv_obj_t*>(a->var);
        lv_obj_add_flag(popup, LV_OBJ_FLAG_HIDDEN);
    }

    // Timeout timer callback
    static void timeout_timer_cb(lv_timer_t * timer) {
        lvgl_popup *instance = static_cast<lvgl_popup*>(lv_timer_get_user_data(timer));
        if (instance) {
            instance->HidePopup();
        }
    }

    void start_timeout_timer(void) {
        if (timeout_timer) {
            lv_timer_del(timeout_timer);
            timeout_timer = nullptr;
        }

        if (timeout_enabled && timeout_ms > 0) {
            timeout_timer = lv_timer_create(timeout_timer_cb, timeout_ms, this);
        }
    }

    void stop_timeout_timer(void) {
        if (timeout_timer) {
            lv_timer_del(timeout_timer);
            timeout_timer = nullptr;
        }
    }

public:
    lvgl_popup(lv_obj_t *parent) {
        if (!parent)
            return;

        // Create popup container - hidden by default
        this->popup = lv_obj_create(parent);
        lv_obj_set_size(this->popup, 0, 0);
        lv_obj_add_flag(this->popup, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(this->popup, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_set_style_bg_color(this->popup, POPUP_BG_COLOR, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(this->popup, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(this->popup, 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(this->popup, POPUP_BORDER_COLOR, LV_PART_MAIN);
        lv_obj_set_style_radius(this->popup, 15, LV_PART_MAIN);

        // Create title label
        this->title_label = lv_label_create(this->popup);
        lv_obj_align(this->title_label, LV_ALIGN_CENTER, 0, -15);
        lv_obj_set_style_text_align(this->title_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_color(this->title_label, POPUP_TEXT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(this->title_label, POPUP_TITLE_FONT, LV_PART_MAIN);
        lv_label_set_text(this->title_label, "");

        // Create content label (scrollable)
        this->content_label = lv_label_create(this->popup);
        lv_obj_align(this->content_label, LV_ALIGN_CENTER, 0, 15);
        lv_obj_set_width(this->content_label, POPUP_WIDTH - 20);
        lv_label_set_long_mode(this->content_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_style_text_align(this->content_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_color(this->content_label, POPUP_TEXT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(this->content_label, POPUP_CONTENT_FONT, LV_PART_MAIN);
        lv_label_set_text(this->content_label, "");
    }

    ~lvgl_popup() {
        stop_timeout_timer();
        if (this->popup) {
            lv_obj_del(this->popup);
            this->popup = nullptr;
        }
    }

    lv_obj_t* GetWidget(void) {
        return this->popup;
    }

    void ShowPopup(std::string title, std::string content, uint32_t timeout_ms = 0) {
        if(this->popup == nullptr)
            return;

        this->title = title;
        this->content = content;
        this->timeout_enabled = (timeout_ms > 0);
        this->timeout_ms = timeout_ms;

        // Update labels
        lv_label_set_text(this->title_label, title.c_str());
        lv_label_set_text(this->content_label, content.c_str());

        // If popup is already visible, do not animate
        if(this->is_visible){
            start_timeout_timer(); // Restart timeout timer
            return;
        }

        // Show widget
        lv_obj_clear_flag(this->popup, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(this->popup, LV_ALIGN_CENTER, 0, 0);

        #ifdef POPUP_ANIMATION_ENABLE
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, this->popup);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)anim_popup_cb);
            lv_anim_set_duration(&a, POPUP_ANIMATION_DURATION);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
            lv_anim_set_values(&a, 0, 100);
            lv_anim_start(&a);
        #else
            lv_obj_set_width(this->popup, POPUP_WIDTH);
            lv_obj_set_height(this->popup, POPUP_HEIGHT);
            lv_obj_set_style_text_opa(this->popup, POPUP_OPACITY, LV_PART_MAIN);
            lv_obj_set_style_bg_opa(this->popup, POPUP_OPACITY, LV_PART_MAIN);
        #endif

        this->is_visible = true;
        start_timeout_timer();
    }

    void HidePopup(void) {
        if(this->popup == nullptr || !is_visible)
            return;

        #ifdef POPUP_ANIMATION_ENABLE
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, this->popup);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)anim_popup_cb);
            lv_anim_set_duration(&a, POPUP_ANIMATION_DURATION);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
            lv_anim_set_values(&a, 100, 0);
            lv_anim_set_completed_cb(&a, anim_completed_cb);
            lv_anim_start(&a);
        #else
            lv_obj_set_width(this->popup, POPUP_WIDTH);
            lv_obj_set_height(this->popup, POPUP_HEIGHT);
            lv_obj_set_style_text_opa(this->popup, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_bg_opa(this->popup, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_add_flag(this->popup, LV_OBJ_FLAG_HIDDEN);
        #endif

        this->is_visible = false;
        stop_timeout_timer();
    }
};