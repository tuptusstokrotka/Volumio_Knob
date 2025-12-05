#pragma once

#include "dev_tools.h"
#include "lvgl.h"
#include "styles/styles.h"
#include "widgets/popup.h"
#include "widgets/icon_button.h"
#include "../notify/CommandQueue.h"
#include <string>

class Dashboard {
private:
    lv_color_t accentColor = ACCENT_COLOR;

    // Screen instance
    lv_obj_t* screen;

    // Screen components
    lv_obj_t* arc;
    lv_obj_t* batteryIcon;

    // Labels
    lv_obj_t* trackTitle;
    lv_obj_t* trackArtist;
    lv_obj_t* trackSamplerate;
    lv_obj_t* trackSeek;

    // Player Icons
    lv_obj_t* playerIcon;
    IconButton* playIcon;
    IconButton* nextIcon;
    IconButton* prevIcon;
    IconButton* repeatIcon;
    IconButton* shuffleIcon;

    // Popup
    lvgl_popup* popup;


    static void arc_anim_exec_cb(void * var, int32_t v) {
        lv_obj_t * arc = static_cast<lv_obj_t*>(var);
        lv_arc_set_value(arc, v);
    }
    void UpdateArc(int value) {
        static int target_arc_value = value;
        int32_t current_value = lv_arc_get_value(this->arc);

        if (current_value == value)
            return;

        #ifdef ARC_ANIMATION_ENABLE
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, this->arc);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)arc_anim_exec_cb);
            lv_anim_set_duration(&a, ARC_ANIMATION_DURATION);
            lv_anim_set_path_cb(&a, lv_anim_path_linear);
            lv_anim_set_values(&a, current_value, value);
            lv_anim_start(&a);
        #else
            lv_arc_set_value(this->arc, value);
        #endif
    }

    static void OnArcTouch(lv_event_t * e) {
        Dashboard * dashboard = static_cast<Dashboard*>(lv_event_get_user_data(e));
        if (dashboard == nullptr) return;

        int arcValue = lv_arc_get_value(dashboard->arc);
        DEBUG_PRINTLN("[Dashboard] Arc touched - Seek to: " << arcValue);
    }

    static void OnPlayIconClick(lv_event_t * e) {
        Dashboard * dashboard = static_cast<Dashboard*>(lv_event_get_user_data(e));
        if (dashboard == nullptr) return;

        DEBUG_PRINTLN("[Dashboard] Play icon clicked");
    }

    static void OnNextIconClick(lv_event_t * e) {
        Dashboard * dashboard = static_cast<Dashboard*>(lv_event_get_user_data(e));
        if (dashboard == nullptr) return;

        DEBUG_PRINTLN("[Dashboard] Next icon clicked");
    }

    static void OnPrevIconClick(lv_event_t * e) {
        Dashboard * dashboard = static_cast<Dashboard*>(lv_event_get_user_data(e));
        if (dashboard == nullptr) return;

        DEBUG_PRINTLN("[Dashboard] Prev icon clicked");
    }

    static void OnRepeatIconClick(lv_event_t * e) {
        Dashboard * dashboard = static_cast<Dashboard*>(lv_event_get_user_data(e));
        if (dashboard == nullptr) return;

        DEBUG_PRINTLN("[Dashboard] Repeat icon clicked");
    }

    static void OnShuffleIconClick(lv_event_t * e) {
        Dashboard * dashboard = static_cast<Dashboard*>(lv_event_get_user_data(e));
        if (dashboard == nullptr) return;

        DEBUG_PRINTLN("[Dashboard] Shuffle icon clicked");
    }

public:
    Dashboard(){
        // Create screen
        screen = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(screen, BG_COLOR, LV_PART_MAIN);

        // Create arc style - WHITE DOT
        static lv_style_t knob_style;
        lv_style_init(&knob_style);
        lv_style_set_radius(&knob_style, LV_RADIUS_CIRCLE);
        lv_style_set_width(&knob_style, 8);
        lv_style_set_bg_color(&knob_style, ARC_KNOB_COLOR);

        // Create arc
        arc = lv_arc_create(screen);
        lv_obj_add_style(arc, &knob_style, LV_PART_KNOB);
        lv_obj_align(arc, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_size(arc, 225, 225);
        lv_obj_set_style_arc_color(arc, ARC_BG_COLOR, LV_PART_MAIN);
        lv_obj_set_style_arc_color(arc, ACCENT_COLOR, LV_PART_INDICATOR);
        lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_clip_corner(arc, true, LV_PART_MAIN);
        lv_arc_set_rotation(arc, 270);
        lv_arc_set_bg_angles(arc, 180+55, 180-55);
        lv_arc_set_mode(arc, LV_ARC_MODE_NORMAL);
        lv_arc_set_range(arc, 0, 100);
        lv_obj_add_event_cb(arc, OnArcTouch, LV_EVENT_CLICKED, this);

        // Create battery icon
        batteryIcon = lv_label_create(screen);
        lv_obj_align(batteryIcon, LV_ALIGN_CENTER, 0, -85);
        lv_obj_set_style_text_color(batteryIcon, TEXT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(batteryIcon, BATTERY_FONT, LV_PART_MAIN);
        lv_label_set_text(batteryIcon, LV_SYMBOL_BATTERY_FULL);

    /* TOP */
        // Create track title label
        trackTitle = lv_label_create(screen);
        lv_obj_align(trackTitle, LV_ALIGN_CENTER, 0, -55);
        lv_obj_set_style_text_color(trackTitle, TEXT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(trackTitle, TITLE_FONT, LV_PART_MAIN);
        lv_obj_set_style_text_align(trackTitle, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_width(trackTitle, 150);
        lv_label_set_text(trackTitle, "Track Title - Long Text to Test the Long Mode");
        lv_label_set_long_mode(trackTitle, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_move_foreground(trackTitle);

        // Create track artist label
        trackArtist = lv_label_create(screen);
        lv_obj_align(trackArtist, LV_ALIGN_CENTER, 0, -35);
        lv_obj_set_style_text_color(trackArtist, ACCENT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(trackArtist, TITLE_FONT, LV_PART_MAIN);
        lv_obj_set_style_text_align(trackArtist, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_width(trackArtist, 160);
        lv_label_set_text(trackArtist, "Artist Name - Long Text to Test the Long Mode");
        lv_label_set_long_mode(trackArtist, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_move_foreground(trackArtist);

    /* MIDDLE */
        // Create play icon
        playIcon = new IconButton(screen);
        playIcon->SetStyle(60, BIG_ICON_FONT);
        playIcon->SetIcon(LV_SYMBOL_PLAY);
        playIcon->SetCallback(OnPlayIconClick, this);
        playIcon->Align(LV_ALIGN_CENTER, 0, 0);

        // Create previous icon
        prevIcon = new IconButton(screen);
        prevIcon->SetStyle(50, SMALL_ICON_FONT);
        prevIcon->SetIcon(LV_SYMBOL_PREV);
        prevIcon->SetCallback(OnPrevIconClick, this);
        prevIcon->Align(LV_ALIGN_CENTER, -60, 0);

        // Create next icon
        nextIcon = new IconButton(screen);
        nextIcon->SetStyle(50, SMALL_ICON_FONT);
        nextIcon->SetIcon(LV_SYMBOL_NEXT);
        nextIcon->SetCallback(OnNextIconClick, this);
        nextIcon->Align(LV_ALIGN_CENTER, 60, 0);

        // Create track samplerate label
        trackSamplerate = lv_label_create(screen);
        lv_obj_align(trackSamplerate, LV_ALIGN_CENTER, 0, 35);
        lv_obj_set_style_text_color(trackSamplerate, TEXT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(trackSamplerate, LABEL_FONT, LV_PART_MAIN);
        lv_obj_set_style_text_align(trackSamplerate, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_label_set_text(trackSamplerate, "44.1 kHz / 16 bit");
        lv_obj_move_foreground(trackSamplerate);

        // Create track seek label
        trackSeek = lv_label_create(screen);
        lv_obj_align(trackSeek, LV_ALIGN_CENTER, 0, 55);
        lv_obj_set_style_text_color(trackSeek, TEXT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(trackSeek, LABEL_FONT, LV_PART_MAIN);
        lv_obj_set_style_text_align(trackSeek, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_label_set_text(trackSeek, "0:00 / 3:45");
        lv_obj_move_foreground(trackSeek);

    /* BOTTOM */
        // Create repeat icon
        repeatIcon = new IconButton(screen);
        repeatIcon->SetStyle(45, SMALL_ICON_FONT);
        repeatIcon->SetIcon(LV_SYMBOL_LOOP);
        repeatIcon->SetCallback(OnRepeatIconClick, this);
        repeatIcon->Align(LV_ALIGN_CENTER, -60, 80);

        // Create shuffle icon
        shuffleIcon = new IconButton(screen);
        shuffleIcon->SetStyle(45, SMALL_ICON_FONT);
        shuffleIcon->SetIcon(LV_SYMBOL_SHUFFLE);
        shuffleIcon->SetCallback(OnShuffleIconClick, this);
        shuffleIcon->Align(LV_ALIGN_CENTER, 60, 80);

        // Create player icon (service/format icon)
        playerIcon = lv_label_create(screen);
        lv_obj_align(playerIcon, LV_ALIGN_CENTER, 0, 100);
        lv_obj_set_style_text_color(playerIcon, ACCENT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(playerIcon, SMALL_ICON_FONT, LV_PART_MAIN);
        lv_label_set_text(playerIcon, LV_SYMBOL_AUDIO);

        // Create popup widget
        popup = new lvgl_popup(screen);
        lv_obj_align(popup->GetWidget(), LV_ALIGN_CENTER, 0, 0);
    }

    ~Dashboard(){
        if (popup) {
            delete popup;
            popup = nullptr;
        }
        if (playIcon) {
            delete playIcon;
            playIcon = nullptr;
        }
        if (prevIcon) {
            delete prevIcon;
            prevIcon = nullptr;
        }
        if (nextIcon) {
            delete nextIcon;
            nextIcon = nullptr;
        }
        if (repeatIcon) {
            delete repeatIcon;
            repeatIcon = nullptr;
        }
        if (shuffleIcon) {
            delete shuffleIcon;
            shuffleIcon = nullptr;
        }
        if (this->screen) {
            lv_obj_del(this->screen);
            this->screen = nullptr;
            arc = nullptr;
        }
    }

    lv_obj_t* GetScreen(void){ return this->screen; }
    lv_obj_t* GetArc(void){ return this->arc; }

    // Volumio event handlers (called from DisplayHandler)
    void OnVolumeChange(int volumeDiff) {
        DEBUG_PRINTLN("[Dashboard] Volume change: " << (volumeDiff > 0 ? "+" : "") << volumeDiff);

        if (popup == nullptr)
            return;

        std::string Title = "Volume";
        std::string Icon = (volumeDiff > 0) ? LV_SYMBOL_VOLUME_MAX : LV_SYMBOL_VOLUME_MID;
        popup->Show(Title, Icon, 1000);
    }

    // Popup
    void ShowPopup(const char *title, const char *content, TickType_t duration = 0){
        if(this->popup == nullptr)
            return;
        this->popup->Show(title, content, duration);
    }
    void HidePopup(void){
        if(this->popup == nullptr)
            return;
        this->popup->Hide();
    }

    // Progress Bar - Arc
    void SetArcValue(int value, int max){
        if(this->arc == nullptr)
            return;
        lv_arc_set_range(this->arc, 0, max);
        UpdateArc(value);
    }

    void SetBatteryValue(int value){
        if(this->batteryIcon == nullptr)
            return;
        lv_obj_set_style_text_color(this->batteryIcon, TEXT_COLOR, LV_PART_MAIN);
        if(value >= 80)
            lv_label_set_text(this->batteryIcon, LV_SYMBOL_BATTERY_FULL);
        else if(value >= 60)
            lv_label_set_text(this->batteryIcon, LV_SYMBOL_BATTERY_3);
        else if(value >= 40)
            lv_label_set_text(this->batteryIcon, LV_SYMBOL_BATTERY_2);
        else if(value >= 20)
            lv_label_set_text(this->batteryIcon, LV_SYMBOL_BATTERY_1);
        else{
            lv_obj_set_style_text_color(this->batteryIcon, lv_color_make(0xFF, 0x00, 0x00), LV_PART_MAIN);
            lv_label_set_text(this->batteryIcon, LV_SYMBOL_BATTERY_EMPTY);
        }
    }

    // Track Info
    void SetTrackTitle(const char *title){
        if(this->trackTitle == nullptr)
            return;
        lv_label_set_text(this->trackTitle, title);
    }
    void SetTrackArtist(const char *artist){
        if(this->trackArtist == nullptr)
            return;
        lv_label_set_text(this->trackArtist, artist);
    }
    void SetTrackSamplerate(const char *samplerate){
        if(this->trackSamplerate == nullptr)
            return;
        lv_label_set_text(this->trackSamplerate, samplerate);
    }
    void SetTrackSeek(int seek, int duration = 0){
        if(this->trackSeek == nullptr)
            return;

        std::string seek_str;
        if(duration > 0){
            int seek_min = seek / 60;
            int seek_sec = seek % 60;
            int dur_min = duration / 60;
            int dur_sec = duration % 60;
            seek_str = std::to_string(seek_min) + ":" +
                      (seek_sec < 10 ? "0" : "") + std::to_string(seek_sec) + " / " +
                      std::to_string(dur_min) + ":" +
                      (dur_sec < 10 ? "0" : "") + std::to_string(dur_sec);
        } else {
            int seek_min = seek / 60;
            int seek_sec = seek % 60;
            seek_str = std::to_string(seek_min) + ":" +
                      (seek_sec < 10 ? "0" : "") + std::to_string(seek_sec);
        }
        lv_label_set_text(this->trackSeek, seek_str.c_str());
    }

    // Player Icons
    void SetPlayerIcon(const char *icon){
        if(this->playerIcon == nullptr)
            return;
        lv_label_set_text(this->playerIcon, icon);
    }
    void SetStatus(bool isPlaying, bool isPaused, bool isStopped){
        if(this->playIcon == nullptr)
            return;
        if(isPlaying)
            playIcon->SetIcon(LV_SYMBOL_PLAY);
        else if(isPaused)
            playIcon->SetIcon(LV_SYMBOL_PAUSE);
        else if(isStopped)
            playIcon->SetIcon(LV_SYMBOL_STOP);
    }
    void SetRepeatIconState(bool enabled){
        if(this->repeatIcon == nullptr)
            return;
        this->repeatIcon->SetIconColor(enabled ? accentColor : TEXT_COLOR);
    }
    void SetShuffleIconState(bool enabled){
        if(this->shuffleIcon == nullptr)
            return;
        this->shuffleIcon->SetIconColor(enabled ? accentColor : TEXT_COLOR);
    }

    // Accent Color
    void SetAccentColor(lv_color_t color){
        accentColor = color;

        if(this->arc != nullptr)
            lv_obj_set_style_arc_color(this->arc, color, LV_PART_INDICATOR);

        if(this->trackArtist != nullptr)
            lv_obj_set_style_text_color(this->trackArtist, color, LV_PART_MAIN);

        if(this->playerIcon != nullptr)
            lv_obj_set_style_text_color(this->playerIcon, color, LV_PART_MAIN);

        // Icon pressed color
        if(this->playIcon != nullptr)
            this->playIcon->SetButtonPressedColor(color);
        if(this->prevIcon != nullptr)
            this->prevIcon->SetButtonPressedColor(color);
        if(this->nextIcon != nullptr)
            this->nextIcon->SetButtonPressedColor(color);
        if(this->repeatIcon != nullptr)
            this->repeatIcon->SetButtonPressedColor(color);
        if(this->shuffleIcon != nullptr)
            this->shuffleIcon->SetButtonPressedColor(color);
    }

};