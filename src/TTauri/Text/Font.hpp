// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/GlyphMetrics.hpp"
#include "TTauri/Text/FontGlyphIDs.hpp"
#include "TTauri/Text/gstring.hpp"
#include "TTauri/Text/FontDescription.hpp"
#include "TTauri/Foundation/Path.hpp"
#include "TTauri/Foundation/ResourceView.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/URL.hpp"
#include <nonstd/span>
#include <vector>
#include <map>

namespace tt {

/*! A font.
 * This class has information on how to shape text and
 * get glyphs consisting of bezier contours.
 */
class Font {
public:
    Font() = default;
    virtual ~Font() = default;
    Font(Font const &) = delete;
    Font &operator=(Font const &) = delete;
    Font(Font &&) = delete;
    Font &operator=(Font &&) = delete;

    /** The description is filled with information parsed from the font.
     */
    FontDescription description;

    /** Get the glyph for a code-point.
     * @return glyph-id, or invalid when not found or error.
     */
    [[nodiscard]] virtual GlyphID find_glyph(char32_t c) const noexcept = 0;

    /** Get the glyphs for a grapheme.
    * @return a set of glyph-ids, or invalid when not found or error.
    */
    [[nodiscard]] FontGlyphIDs find_glyph(Grapheme g) const noexcept;

    /*! Load a glyph into a path.
    * The glyph is directly loaded from the font file.
    * 
    * \param glyphIndex the index of a glyph inside the font.
    * \param path The path constructed by the loader.
    * \return empty on failure, or the glyphID of the metrics to use.
    */
    virtual std::optional<GlyphID> loadGlyph(GlyphID glyph_id, Path &path) const noexcept = 0;

    /*! Load a glyph into a path.
    * The glyph is directly loaded from the font file.
    * 
    * \param glyphIndex the index of a glyph inside the font.
    * \param metrics The metrics constructed by the loader.
    * \return true on success, false on error.
    */
    virtual bool loadGlyphMetrics(GlyphID glyph_id, GlyphMetrics &metrics, GlyphID lookahead_glyph_id=GlyphID{}) const noexcept = 0;
};

}

namespace tt {

template<>
std::unique_ptr<tt::Font> parseResource(URL const &location);

}
