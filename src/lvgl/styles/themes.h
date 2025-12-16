#pragma once
#include <unordered_map>
#include <string>
#include <string_view>
#include "../font/IconsFontAwesome4.h"
#include "styles.h"

struct Theme {
    lv_color_t color;
    const char* icon;
};

inline const std::unordered_map<std::string_view, Theme> themes = {
    { "spotify", {lv_color_make(0x1E, 0xD7, 0x60), ICON_FA_SPOTIFY }         },
    { "youtube", {lv_color_make(0xFF, 0x00, 0x00), ICON_FA_YOUTUBE_PLAY }    },
    { "airplay", {lv_color_make(0x00, 0x66, 0xCC), ICON_FA_APPLE }           },
};

inline const Theme default_theme = {
    ACCENT_COLOR,
    ICON_FA_MUSIC
};

inline const Theme* get_theme(std::string_view name) {
    auto it = themes.find(name);
    return (it != themes.end()) ? &it->second : nullptr;
}