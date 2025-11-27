#pragma once

#include "lvgl.h"
#include "styles.h"
#include <string>

#define ANIMATION_ENABLE
#define ANIMATION_DURATION 200 // [ms]

class Dashboard {
    lv_color_t accentColor = ACCENT_COLOR;

    // Screen instance
    lv_obj_t* screen;

    // Screen components
    lv_obj_t* arc;
    lv_obj_t* batteryIcon;

    // Labels
    lv_obj_t* trackTitle;
    lv_obj_t* trackArtist;
    lv_obj_t* trackSamplerate;  // lv_obj_t *trackBitdepth; // Combined with samplerate
    lv_obj_t* trackSeek;        // lv_obj_t *trackDuration; // Combined with seek

    // Player Icons
    lv_obj_t* playerIcon;
    lv_obj_t* playIcon;
    lv_obj_t* nextIcon;
    lv_obj_t* pauseIcon;
    lv_obj_t* prevIcon;
    lv_obj_t* repeatIcon;
    lv_obj_t* shuffleIcon;

    // Target value for animation
    int target_arc_value = 0;

    // Animation execution callback - updates arc value during animation
    static void arc_anim_exec_cb(void * var, int32_t v) {
        // Get arc object
        lv_obj_t * arc = static_cast<lv_obj_t*>(var);
        lv_arc_set_value(arc, v);
    }

    // Animate arc to a specific value
    void UpdateArc(int value) {
        // Store target value
        target_arc_value = value;

        // Get current value
        int32_t current_value = lv_arc_get_value(this->arc);

        // Update if there's a difference
        if (current_value == value)
            return;

        #ifdef ANIMATION_ENABLE
            // Create animation
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, this->arc);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)arc_anim_exec_cb);
            lv_anim_set_duration(&a, ANIMATION_DURATION);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
            lv_anim_set_values(&a, current_value, value);
            lv_anim_start(&a);
        #else
            lv_arc_set_value(this->arc, value);
        #endif
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
        lv_style_set_width(&knob_style, 8); // Required to show knob
        lv_style_set_bg_color(&knob_style, lv_color_make(0xFF, 0xFF, 0xFF));

        // Create arc
        arc = lv_arc_create(screen);
        lv_obj_add_style(arc, &knob_style, LV_PART_KNOB);
        lv_obj_align(arc, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_size(arc, 225, 225);
        lv_obj_set_style_arc_color(arc, lv_color_make(0x20, 0x20, 0x20), LV_PART_MAIN);
        lv_obj_set_style_arc_color(arc, ACCENT_COLOR, LV_PART_INDICATOR);
        lv_arc_set_rotation(arc, 270);
        lv_arc_set_bg_angles(arc, 180+50, 180-50);
        lv_arc_set_mode(arc, LV_ARC_MODE_NORMAL);
        lv_arc_set_range(arc, 0, 100);
        lv_arc_set_value(arc, 25);

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

        // Create track artist label
        trackArtist = lv_label_create(screen);
        lv_obj_align(trackArtist, LV_ALIGN_CENTER, 0, -35);
        lv_obj_set_style_text_color(trackArtist, ACCENT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(trackArtist, TITLE_FONT, LV_PART_MAIN);
        lv_obj_set_style_text_align(trackArtist, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_width(trackArtist, 160);
        lv_label_set_text(trackArtist, "Artist Name - Long Text to Test the Long Mode");
        lv_label_set_long_mode(trackArtist, LV_LABEL_LONG_SCROLL_CIRCULAR);



    /* MIDDLE */
        // Create play icon
        playIcon = lv_label_create(screen);
        lv_obj_align(playIcon, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_color(playIcon, TEXT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(playIcon, BIG_ICON_FONT, LV_PART_MAIN);
        lv_label_set_text(playIcon, LV_SYMBOL_PLAY);
        // Create pause icon
        pauseIcon = lv_label_create(screen);
        lv_obj_align(pauseIcon, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_color(pauseIcon, TEXT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(pauseIcon, BIG_ICON_FONT, LV_PART_MAIN);
        lv_label_set_text(pauseIcon, LV_SYMBOL_PAUSE);
        lv_obj_add_flag(pauseIcon, LV_OBJ_FLAG_HIDDEN);

        // Create previous icon
        prevIcon = lv_label_create(screen);
        lv_obj_align(prevIcon, LV_ALIGN_CENTER, -60, 0);
        lv_obj_set_style_text_color(prevIcon, TEXT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(prevIcon, BIG_ICON_FONT, LV_PART_MAIN);
        lv_label_set_text(prevIcon, LV_SYMBOL_PREV);

        // Create next icon
        nextIcon = lv_label_create(screen);
        lv_obj_align(nextIcon, LV_ALIGN_CENTER, 60, 0);
        lv_obj_set_style_text_color(nextIcon, TEXT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(nextIcon, BIG_ICON_FONT, LV_PART_MAIN);
        lv_label_set_text(nextIcon, LV_SYMBOL_NEXT);


        // Create track samplerate label
        trackSamplerate = lv_label_create(screen);
        lv_obj_align(trackSamplerate, LV_ALIGN_CENTER, 0, 35);
        lv_obj_set_style_text_color(trackSamplerate, TEXT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(trackSamplerate, LABEL_FONT, LV_PART_MAIN);
        lv_obj_set_style_text_align(trackSamplerate, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_label_set_text(trackSamplerate, "44.1 kHz / 16 bit");

        // Create track seek label
        trackSeek = lv_label_create(screen);
        lv_obj_align(trackSeek, LV_ALIGN_CENTER, 0, 55);
        lv_obj_set_style_text_color(trackSeek, TEXT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(trackSeek, LABEL_FONT, LV_PART_MAIN);
        lv_obj_set_style_text_align(trackSeek, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_label_set_text(trackSeek, "0:00 / 3:45");


    /* BOTTOM */
        // Create repeat icon
        repeatIcon = lv_label_create(screen);
        lv_obj_align(repeatIcon, LV_ALIGN_CENTER, -60, 80);
        lv_obj_set_style_text_color(repeatIcon, TEXT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(repeatIcon, SMALL_ICON_FONT, LV_PART_MAIN);
        lv_label_set_text(repeatIcon, LV_SYMBOL_REFRESH);
        lv_obj_add_state(repeatIcon, LV_STATE_DISABLED);

        // Create shuffle icon
        shuffleIcon = lv_label_create(screen);
        lv_obj_align(shuffleIcon, LV_ALIGN_CENTER, 60, 80);
        lv_obj_set_style_text_color(shuffleIcon, TEXT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(shuffleIcon, SMALL_ICON_FONT, LV_PART_MAIN);
        lv_label_set_text(shuffleIcon, LV_SYMBOL_SHUFFLE);
        lv_obj_add_state(shuffleIcon, LV_STATE_DISABLED);

        // Create player icon (service/format icon)
        playerIcon = lv_label_create(screen);
        lv_obj_align(playerIcon, LV_ALIGN_CENTER, 0, 100);
        lv_obj_set_style_text_color(playerIcon, ACCENT_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(playerIcon, SMALL_ICON_FONT, LV_PART_MAIN);
        lv_label_set_text(playerIcon, LV_SYMBOL_AUDIO);
    }

    ~Dashboard(){
        if (this->screen) {
            lv_obj_del(this->screen);
            this->screen = nullptr;
            arc = nullptr;
        }
    }

    lv_obj_t* GetScreen(void){ return this->screen; }

    // Arc
    void SetRange(int min, int max){
        if(this->arc == nullptr)
            return;
        lv_arc_set_range(this->arc, min, max);
    }

    void SetArcValue(int value){
        if(this->arc == nullptr)
            return;
        UpdateArc(value);
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
    void SetTrackSeek(const char *seek){
        if(this->trackSeek == nullptr)
            return;
        lv_label_set_text(this->trackSeek, seek);
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
    void SetPlayIconVisible(bool visible){
        if(this->playIcon == nullptr)
            return;
        if(visible)
            lv_obj_clear_flag(this->playIcon, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(this->playIcon, LV_OBJ_FLAG_HIDDEN);
    }
    void SetPauseIconVisible(bool visible){
        if(this->pauseIcon == nullptr)
            return;
        if(visible)
            lv_obj_clear_flag(this->pauseIcon, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(this->pauseIcon, LV_OBJ_FLAG_HIDDEN);
    }
    void SetNextIconVisible(bool visible){
        if(this->nextIcon == nullptr)
            return;
        if(visible)
            lv_obj_clear_flag(this->nextIcon, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(this->nextIcon, LV_OBJ_FLAG_HIDDEN);
    }
    void SetPrevIconVisible(bool visible){
        if(this->prevIcon == nullptr)
            return;
        if(visible)
            lv_obj_clear_flag(this->prevIcon, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(this->prevIcon, LV_OBJ_FLAG_HIDDEN);
    }
    void SetRepeatIconState(bool enabled){
        if(this->repeatIcon == nullptr)
            return;
        if(enabled)
            lv_obj_clear_state(this->repeatIcon, LV_STATE_DISABLED);
        else
            lv_obj_add_state(this->repeatIcon, LV_STATE_DISABLED);
    }
    void SetShuffleIconState(bool enabled){
        if(this->shuffleIcon == nullptr)
            return;
        if(enabled)
            lv_obj_clear_state(this->shuffleIcon, LV_STATE_DISABLED);
        else
            lv_obj_add_state(this->shuffleIcon, LV_STATE_DISABLED);
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
    }

};