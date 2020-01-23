// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/theme.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include "TTauri/Foundation/geometry.hpp"
#include "TTauri/Foundation/GlyphMetrics.hpp"
#include "TTauri/Foundation/gstring.hpp"
#include "TTauri/Foundation/Path.hpp"
#include <string_view>

namespace TTauri {

struct AttributedGrapheme {
    Grapheme grapheme;
    int index;

    /** All information about the shape and color needed to render this grapheme. */
    TextStyle style;

    AttributedGrapheme(Grapheme grapheme, int index, TextStyle style) :
        grapheme(std::move(grapheme)), index(index), style(std::move(style)) {}
};

/**
 */
struct AttributedGlyph {
    FontGlyphIDs glyphs;

    Grapheme grapheme;

    /** Copied from the original attributed-grapheme.
     * An attributed-glyph always represents one or more (ligature) graphemes, a grapheme is never split.
     */
    int index;

    /** Number of graphemes merged (ligature) into this attributed-glyph. */
    uint8_t grapheme_count;

    /** Copied from the original attributed-grapheme. */
    TextStyle style;

    /** Metrics taken from the font file. */
    GlyphMetrics metrics;

    glm::vec2 position;

    AttributedGlyph(AttributedGrapheme const &attr_grapheme, FontGlyphIDs glyphs) noexcept :
        glyphs(std::move(glyphs)), grapheme(attr_grapheme.grapheme), index(attr_grapheme.index), style(attr_grapheme.style), metrics() {}
};

/** ShapedText represent a piece of text shaped to be displayed.
 */
class ShapedText {
    std::vector<AttributedGlyph> text;
    extent2 box_size;

public:
    ShapedText(std::vector<AttributedGrapheme> const &text, Alignment const &alignment, extent2 const &minimum_size, extent2 const &maximum_size) noexcept;
    ShapedText(gstring const &text, TextStyle const &style, Alignment const &alignment, extent2 const &minimum_size, extent2 const &maximum_size) noexcept;
    ShapedText(std::string const &text, TextStyle const &style, Alignment const &alignment, extent2 const &minimum_size, extent2 const &maximum_size) noexcept;

    ShapedText(ShapedText const &other) noexcept = default;
    ShapedText(ShapedText &&other) noexcept = default;
    ShapedText &operator=(ShapedText const &other) noexcept = default;
    ShapedText &operator=(ShapedText &&other) noexcept = default;
    ~ShapedText() = default;

    /** Convert the whole shaped text into a layered path.
     */
    [[nodiscard]] Path toPath() const noexcept;
};



}