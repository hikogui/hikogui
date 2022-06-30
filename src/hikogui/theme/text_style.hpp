

#pragma once

#include "../i18n/language_tag.hpp"

namespace hi::inline v1 {

struct font_style {
    language_tag language_filter;
    font_family_id family_id;
    font_variant variant;
    float size;
};

class text_style_proper {
public:

private:
    color _foreground;
    color _background;
    std::vector<font_style> _font_styles;
};

inline auto text_styles = stable_set<text_style_proper>{};

class text_style {
public:


private:
    uint16_t _id;
};


}

