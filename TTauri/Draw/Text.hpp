// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "PixelMap.hpp"
#include "Theme.hpp"
#include "TTauri/Color.hpp"
#include "TTauri/strings.hpp"

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

    std::string setFontIndex(uint8_t index) {
        fontIndex = index;
        return translateString<std::string>(std::u32string(1, TEXT_STYLE_FONT_INDEX_BEGIN + index));
    }

    std::string setFontSize(uint8_t size) {
        fontSize = size;
        return translateString<std::string>(std::u32string(1, TEXT_STYLE_FONT_SIZE_BEGIN + size));
    }

    std::string setColorIndex(uint8_t index) {
        colorIndex = index;
        return translateString<std::string>(std::u32string(1, TEXT_STYLE_COLOR_INDEX_BEGIN + index));
    }

    std::string setUnderline(bool x) {
        underline = x;
        return translateString<std::string>(std::u32string(1, x ? TEXT_STYLE_UNDERLINE_ON : TEXT_STYLE_UNDERLINE_OFF));
    }

    std::string setStrikeThrough(bool x) {
        strikeThrough = x;
        return translateString<std::string>(std::u32string(1, x ? TEXT_STYLE_STRIKE_THROUGH_ON : TEXT_STYLE_STRIKE_THROUGH_OFF));
    }

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

const uint64_t GRAPHEME_BEYOND_UNICODE = 0x11'0000; // Only 17 planes (65536 code points)
const uint64_t GRAPHEME_BEYOND_CLUSTER_INDEX = 0x0e'ffff; // Use the other 15 planes for cluster indices.
const uint64_t GRAPHEME_CODE_POINT_BITS = 0x1f'ffff;
const uint64_t GRAPHEME_CODE_POINT_MASK = ~GRAPHEME_CODE_POINT_BITS;

const int GRAPHEME_LIGATURE_BIT = 21;
const uint64_t GRAPHEME_LIGATURE_BITS = 1ULL << GRAPHEME_LIGATURE_BIT;
const uint64_t GRAPHEME_LIGATURE_MASK = ~GRAPHEME_LIGATURE_BITS;

const int GRAPHEME_STYLE_BIT = 22;
const uint64_t GRAPHEME_STYLE_BITS = 1ULL << GRAPHEME_STYLE_BIT;
const uint64_t GRAPHEME_STYLE_MASK = ~GRAPHEME_STYLE_BITS;

const int GRAPHEME_GLYPH_INDEX_BIT = 48;
const uint64_t GRAPHEME_GLYPH_INDEX_BITS = 0xffffULL << GRAPHEME_GLYPH_INDEX_BIT;
const uint64_t GRAPHEME_GLYPH_INDEX_MASK = ~GRAPHEME_GLYPH_INDEX_BITS;

/*! A single grapheme.
 */
struct Grapheme {
    /*! value of the grapheme
     * Bits    Description
     * [20: 0] Unicode code point, or index into a grapheme cluster table. (21 bits)
     * [21:21] Forms ligature with previous grapheme (may be set on multiple contigues graphemes). (1 bit)
     * [47:22] Grapheme style (26 bits)
     * [63:48] Glyph index. (16 bits)
     */
    uint64_t value;

    Grapheme() : value(0) {}

    Grapheme(char32_t codePoint, TextStyle style) : value(0) {
        setCodePoint(codePoint);
        setStyle(style);
    }

    bool isSingleCodePoint() const {
        return (value & GRAPHEME_CODE_POINT_BITS) < GRAPHEME_BEYOND_UNICODE;
    }

    char32_t codePoint() const {
        assert(isSingleCodePoint());
        return value & GRAPHEME_CODE_POINT_BITS;
    }

    void setCodePoint(char32_t codePoint) {
        assert(codePoint < GRAPHEME_BEYOND_UNICODE);
        value = (value & GRAPHEME_CODE_POINT_MASK) | codePoint;
    }

    size_t clusterIndex() const {
        assert(!isSingleCodePoint());
        return (value & GRAPHEME_CODE_POINT_BITS) - GRAPHEME_BEYOND_UNICODE;
    }

    void setClusterIndex(size_t index) {
        assert(index < GRAPHEME_BEYOND_CLUSTER_INDEX);
        value = (value & GRAPHEME_CODE_POINT_MASK) | (index + GRAPHEME_BEYOND_UNICODE);
    }

    bool previousIsLigature() const {
        return (value & GRAPHEME_LIGATURE_BITS) > 0;
    }

    void setPreviousIsLigature(bool x) {
        if (x) {
            value |= GRAPHEME_LIGATURE_BITS;
        } else {
            value &= GRAPHEME_LIGATURE_MASK;
        }
    }

    uint16_t getGlyphIndex() const {
        return (value & GRAPHEME_GLYPH_INDEX_BITS) >> GRAPHEME_GLYPH_INDEX_BIT;
    }

    void setGlyphIndex(uint16_t index) {
        value = (value & GRAPHEME_GLYPH_INDEX_MASK) | (static_cast<uint64_t>(index) << GRAPHEME_GLYPH_INDEX_BIT);
    }

    TextStyle getStyle() const {
        return {(value & GRAPHEME_STYLE_BITS) >> GRAPHEME_STYLE_BIT};
    }

    void setStyle(TextStyle style) {
        value = (value & GRAPHEME_STYLE_MASK) | (static_cast<uint64_t>(style.getPacked()) << GRAPHEME_STYLE_BIT);
    }

    std::pair<uint16_t,uint8_t> getGlyph() const {
        return { getGlyphIndex(), getStyle().fontIndex };
    }

    void setGlyph(uint16_t glyphIndex, uint8_t fontIndex) {
        auto style = getStyle();
        style.fontIndex = fontIndex;
        setStyle(style);
        setGlyphIndex(glyphIndex);
    }

    bool operator==(Grapheme const &other) const {
        return value == other.value;
    }

    static Grapheme GraphemeFromClusterIndex(size_t clusterIndex, TextStyle style) {
        Grapheme r;
        r.setStyle(style);
        r.setClusterIndex(clusterIndex);
        return r;
    }
};

using GraphemeCluster = std::vector<Grapheme>;

struct Text {
    std::vector<GraphemeCluster> clusters;
    std::vector<Grapheme> text;

    Text(std::string const& str, Theme const &theme=*theme);

    void addGraphemeCluster(GraphemeCluster const& cluster);

    void render(PixelMap<uint32_t> pixels, glm::vec2 offset, float angle);

};

}