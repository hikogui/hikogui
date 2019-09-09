// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Path.hpp"

namespace TTauri::Draw {

class TrueTypeFont {
private:
    gsl::span<std::byte const> bytes;

    //! 'cmap' character to glyph mapping
    gsl::span<std::byte const> cmapTableBytes;

    //! The bytes of a unicode character map.
    gsl::span<std::byte const> cmapBytes;

    //! 'glyf' glyph data
    gsl::span<std::byte const> glyfTableBytes;

    //! 'head' font header
    gsl::span<std::byte const> headTableBytes;

    //! 'hhea' horizontal header
    gsl::span<std::byte const> hheaTableBytes;

    //! 'hmtx' horizontal metrics
    gsl::span<std::byte const> hmtxTableBytes;

    //! 'loca' index to location
    gsl::span<std::byte const> locaTableBytes;

    //! 'maxp' maximum profile
    gsl::span<std::byte const> maxpTableBytes;

    //! 'name' naming (not needed)
    gsl::span<std::byte const> nameTableBytes;

    //! 'post' PostScript (not needed)
    gsl::span<std::byte const> postTableBytes;

public:
    /*! Load a true type font.
     * The methods in this class will parse the true-type font at run time.
     * This also means that the bytes passed into this constructor will need to
     * remain available.
     */
    TrueTypeFont(gsl::span<std::byte const> bytes);

    /*! Find a glyph in the font based on an unicode code-point.
     * This is seperated from loading a glyph so that graphemes and ligatures can be found.
     *
     * \param c Unicode code point to look up.
     * \return a glyph-index if a glyph has been found. glyph 0 means "not found".
     */
    int searchCharacterMap(char32_t c) const noexcept;

    /*! Load a glyph into a path.
     * The glyph is directly loaded from the font file.
     * 
     * \param glyphIndex the index of a glyph inside the font.
     * \return A path representing the glyph.
     */
    Path loadGlyph(int glyphIndex) const noexcept;

private:
    void parseFontDirectory();

};

}

