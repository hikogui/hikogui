// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace TTauri::Draw {

const char32_t TEXT_STYLE_FONT_INDEX_BEGIN = 0x10'8800;
const char32_t TEXT_STYLE_FONT_INDEX_END = 0x10'8900;
const char32_t TEXT_STYLE_FONT_SIZE_BEGIN = 0x10'8900;
const char32_t TEXT_STYLE_FONT_SIZE_END = 0x10'8a00;
const char32_t TEXT_STYLE_COLOR_INDEX_BEGIN = 0x10'8a00;
const char32_t TEXT_STYLE_COLOR_INDEX_END = 0x10'8b00;
const char32_t TEXT_STYLE_UNDERLINE_OFF = 0x10'8b00;
const char32_t TEXT_STYLE_UNDERLINE_ON = 0x10'8b01;
const char32_t TEXT_STYLE_STRIKE_THROUGH_ON = 0x10'8b02;
const char32_t TEXT_STYLE_STRIKE_THROUGH_OFF = 0x10'8b03;

/*!
 * The packed variant must fit within 26 bits.
 * 26 = 64 - 21 (unicode code point) - 16 (glyph index) - 1 (ligature marker)
 */
struct TextStyle {
    uint8_t fontIndex;
    uint8_t fontSize;
    uint8_t colorIndex;
    bool underline;
    bool strikeThrough;

    TextStyle() : fontIndex(0), fontSize(0), colorIndex(0), underline(false), strikeThrough(false) {}
    TextStyle(uint32_t packed) :
        fontIndex(packed & 0xff),
        fontSize((packed >> 8) & 0xff),
        colorIndex((packed >> 16) & 0xff),
        underline((packed & 0x1'00'00'00) > 0),
        strikeThrough((packed & 0x2'00'00'00) > 0) {}

    bool updateFromCodePoint(char32_t codePoint) {
        if (codePoint >= TEXT_STYLE_FONT_INDEX_BEGIN && codePoint < TEXT_STYLE_FONT_INDEX_END) {
            fontIndex = (codePoint - TEXT_STYLE_FONT_INDEX_BEGIN) & 0xff;
            return true;
        }
        if (codePoint >= TEXT_STYLE_FONT_SIZE_BEGIN && codePoint < TEXT_STYLE_FONT_SIZE_END) {
            fontSize = (codePoint - TEXT_STYLE_FONT_SIZE_BEGIN) & 0xff;
            return true;
        }
        if (codePoint >= TEXT_STYLE_COLOR_INDEX_BEGIN && codePoint < TEXT_STYLE_COLOR_INDEX_END) {
            colorIndex = (codePoint - TEXT_STYLE_COLOR_INDEX_BEGIN) & 0xff;
            return true;
        }
        switch (codePoint) {
        case TEXT_STYLE_UNDERLINE_OFF:
            underline = false;
            return true;
        case TEXT_STYLE_UNDERLINE_ON:
            underline = true;
            return true;
        case TEXT_STYLE_STRIKE_THROUGH_OFF:
            strikeThrough = false;
            return true;
        case TEXT_STYLE_STRIKE_THROUGH_ON:
            strikeThrough = true;
            return true;
        }
        return false;
    }

    uint32_t getPacked() const {
        return (
            static_cast<uint32_t>(fontIndex) |
            (static_cast<uint32_t>(fontSize) << 8) |
            (static_cast<uint32_t>(colorIndex) << 16) |
            (static_cast<uint32_t>(underline) << 24) |
            (static_cast<uint32_t>(strikeThrough) << 25)
            );
    }
};

}