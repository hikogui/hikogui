// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Draw/Path.hpp"
#include "TTauri/Draw/Font.hpp"
#include "TTauri/Foundation/ResourceView.hpp"
#include <memory>

namespace TTauri::Draw {

class TrueTypeFont final: public Font {
private:
    std::unique_ptr<ResourceView> view;

    gsl::span<std::byte const> bytes;

    float xHeight;
    float HHeight;

    //! 'cmap' character to glyph mapping
    gsl::span<std::byte const> cmapTableBytes;

    //! The bytes of a unicode character map.
    gsl::span<std::byte const> cmapBytes;

    //! 'glyf' glyph data
    gsl::span<std::byte const> glyfTableBytes;

    //! 'head' font header
    gsl::span<std::byte const> headTableBytes;
    float unitsPerEm;
    float emScale;
    bool locaTableIsOffset32;

    //! 'hhea' horizontal header
    gsl::span<std::byte const> hheaTableBytes;
    float ascender;
    float descender;
    int16_t numberOfHMetrics;

    //! 'hmtx' horizontal metrics
    gsl::span<std::byte const> hmtxTableBytes;

    //! 'loca' index to location
    gsl::span<std::byte const> locaTableBytes;

    //! 'maxp' maximum profile
    gsl::span<std::byte const> maxpTableBytes;
    int numGlyphs;

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
    TrueTypeFont(gsl::span<std::byte const> bytes) :
        bytes(bytes) {
        parseFontDirectory();
    }

    TrueTypeFont(ResourceView const &view) :
        view(std::make_unique<ResourceView>(view)), bytes(view.bytes()) {
        parseFontDirectory();
    }


    TrueTypeFont() = delete;
    TrueTypeFont(TrueTypeFont const &other) = delete;
    TrueTypeFont &operator=(TrueTypeFont const &other) = delete;
    TrueTypeFont(TrueTypeFont &&other) = delete;
    TrueTypeFont &operator=(TrueTypeFont &&other) = delete;
    ~TrueTypeFont() = default;

    /*! Find a glyph in the font based on an unicode code-point.
     * This is seperated from loading a glyph so that graphemes and ligatures can be found.
     *
     * \param c Unicode code point to look up.
     * \return a glyph-index if a glyph has been found. 0 means "not found", -1 means "parse error".
     */
    int searchCharacterMap(char32_t c) const noexcept override;

    /*! Load a glyph into a path.
     * The glyph is directly loaded from the font file.
     * 
     * \param glyphIndex the index of a glyph inside the font.
     * \param path The path constructed by the loader.
     * \return 1 on success, 0 on not implemented, -1 on parse error, -2 argument error.
     */
    bool loadGlyph(int glyphIndex, Path &path) const noexcept override;

private:
    /*! Parses the directory table of the font file.
     * This function is called by the constructor to set up references
     * inside the file for each table.
     */
    void parseFontDirectory();

    /*! Parses the head table of the font file.
     * This function is called by parseFontDirectory().
     */
    void parseHeadTable(gsl::span<std::byte const> headTableBytes);

    void parseHHEATable(gsl::span<std::byte const> bytes);

    /*! Parses the maxp table of the font file.
    * This function is called by parseFontDirectory().
    */
    void parseMaxpTable(gsl::span<std::byte const> bytes);

    /*! Find the glyph in the loca table.
     * called by loadGlyph()
     */
    bool getGlyphBytes(int glyphIndex, gsl::span<std::byte const> &bytes) const noexcept;

    /*! Update the loaded glyph with metrics from the font tables.
     * called by loadGlyph()
     */
    bool updateGlyphMetrics(int glyphIndex, Path &glyph) const noexcept;

    bool loadSimpleGlyph(gsl::span<std::byte const> bytes, Path &glyph) const noexcept;

    /*! Load a compound glyph.
     * This will call loadGlyph() recursively.
     *
     * \param bytes Bytes inside the glyf table of this specific compound glyph.
     * \param glyph The path to update with points from the subglyphs.
     * \param metricsGlyphIndex The glyph index of the glyph to use for the metrics.
     *                          this value is only updated when the USE_MY_METRICS flag was set.
     */
    bool loadCompoundGlyph(gsl::span<std::byte const> bytes, Path &glyph, uint16_t &metricsGlyphIndex) const noexcept;
};

}

