#pragma once

/* DASHBOARD */
    // Fonts
    #define BATTERY_FONT        &lv_font_montserrat_18
    #define BIG_ICON_FONT       &lv_font_montserrat_32
    #define SMALL_ICON_FONT     &lv_font_montserrat_24
    #define TITLE_FONT          &lv_font_montserrat_18_latin
    #define LABEL_FONT          &lv_font_montserrat_14
    #define PLAYER_ICONS        &lv_icons_fontawesome_24

    // Colors
    #define BG_COLOR            lv_color_make(0x00, 0x00, 0x00)
    #define ACCENT_COLOR        lv_color_make(0xFF, 0xA0, 0x00)
    #define TEXT_COLOR          lv_color_make(0xFF, 0xFF, 0xFF)

    // Dashboard colors
    #define ARC_KNOB_COLOR      lv_color_make(0xFF, 0xFF, 0xFF)
    #define ARC_BG_COLOR        lv_color_make(0x20, 0x20, 0x20)

    // Animations
    #define ARC_ANIMATION_ENABLE    1
    #define ARC_ANIMATION_DURATION  250


/* POPUP */
    #define POPUP_WIDTH         200
    #define POPUP_HEIGHT        120

    // Fonts
    #define POPUP_TITLE_FONT    &lv_font_montserrat_24
    #define POPUP_CONTENT_FONT  &lv_font_montserrat_18

    // Colors
    #define POPUP_BG_COLOR      lv_color_make(0x20, 0x20, 0x20)
    #define POPUP_BORDER_COLOR  lv_color_make(0xAF, 0xAF, 0xAF)
    #define POPUP_TEXT_COLOR    lv_color_make(0xFF, 0xFF, 0xFF)
    #define POPUP_OPACITY       LV_OPA_90

    // Animations
    #define POPUP_ANIMATION_ENABLE
    #define POPUP_ANIMATION_DURATION 350