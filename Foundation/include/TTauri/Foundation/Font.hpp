// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/Path.hpp"
#include "TTauri/Foundation/PathString.hpp"
#include "TTauri/Foundation/gstring.hpp"
#include "TTauri/Foundation/ResourceView.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/GlyphMetrics.hpp"
#include "TTauri/Foundation/FontDescription.hpp"
#include <vector>
#include <map>
#include <gsl/gsl>

namespace TTauri {

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
    [[nodiscard]] FontGlyphIDs find_glyph(grapheme g) const noexcept;

    /*! Load a glyph into a path.
    * The glyph is directly loaded from the font file.
    * 
    * \param glyphIndex the index of a glyph inside the font.
    * \param path The path constructed by the loader.
    * \return true on success, false on error.
    */
    virtual bool loadGlyph(int glyphIndex, Path &path) const noexcept = 0;

    /*! Load a glyph into a path.
    * The glyph is directly loaded from the font file.
    * 
    * \param glyphIndex the index of a glyph inside the font.
    * \param metrics The metrics constructed by the loader.
    * \return true on success, false on error.
    */
    virtual bool loadGlyphMetrics(int glyphIndex, GlyphMetrics &metrics) const noexcept = 0;

    PathString getGlyphs(gstring const &graphemes) const noexcept {
        PathString r;

        for (int graphemeIndex = 0; graphemeIndex < graphemes.size(); graphemeIndex++) {
            let &grapheme = graphemes.at(graphemeIndex);

            // First try composed normalization
            std::vector<Path> graphemeGlyphs;
            for (let c: grapheme.NFC()) {
                let glyph_id = find_glyph(c);
                if (!glyph_id) {
                    // The codePoint was not found in the font, or a parse error occurred.
                    graphemeGlyphs.clear();
                    break;
                }

                Path glyph;
                if (!loadGlyph(glyph_id, glyph)) {
                    // Some kind of parsing error, causes the glyph not to be loaded.
                    graphemeGlyphs.clear();
                    break;
                }

                graphemeGlyphs.push_back(std::move(glyph));
            }

            if (graphemeGlyphs.size() == 0) {
                // Try again with decomposed normalization.
                for (let c: grapheme.NFD()) {
                    let glyph_id = find_glyph(c);
                    if (!glyph_id) {
                        graphemeGlyphs.clear();
                        break;
                    }

                    Path glyph;
                    if (!loadGlyph(glyph_id, glyph)) {
                        // Some kind of parsing error, causes the glyph not to be loaded.
                        graphemeGlyphs.clear();
                        break;
                    }

                    graphemeGlyphs.push_back(std::move(glyph));
                }
            }

            if (graphemeGlyphs.size() == 0) {
                // Replace with not-found-glyph at index 0.
                Path glyph;
                if (!loadGlyph(0, glyph)) {
                    // Some kind of parsing error, causes the glyph not to be loaded.
                    LOG_FATAL("Could not load glyph 0 from font file.");
                }

                graphemeGlyphs.push_back(std::move(glyph));
            }

            // Add graphemeGlyphs.
            for (let glyph: graphemeGlyphs) {
                r.add(std::move(glyph));
            }
        }

        return r;
    }

    PathString getGlyphs(std::string const &s) const noexcept {
        return getGlyphs(translateString<gstring>(s));
    }
};

}

namespace TTauri {

template<>
std::unique_ptr<Font> parseResource(URL const &location);

}
