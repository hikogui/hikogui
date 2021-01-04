// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "Font.hpp"
#include "../Path.hpp"
#include "../ResourceView.hpp"
#include "../URL.hpp"
#include "../error_info.hpp"
#include <memory>

namespace tt {

class TrueTypeFont final: public Font {
private:
    std::unique_ptr<ResourceView> view;

    std::span<std::byte const> file_bytes;

    uint16_t OS2_xHeight = 0;
    uint16_t OS2_HHeight = 0;

    /// 'cmap' character to glyph mapping
    std::span<std::byte const> cmapTableBytes;

    /// The bytes of a Unicode character map.
    std::span<std::byte const> cmapBytes;

    /// 'glyf' glyph data
    std::span<std::byte const> glyfTableBytes;

    /// 'head' font header
    std::span<std::byte const> headTableBytes;
    float unitsPerEm;
    float emScale;
    bool locaTableIsOffset32;

    /// 'hhea' horizontal header
    std::span<std::byte const> hheaTableBytes;
    float ascender;
    float descender;
    float lineGap;
    uint16_t numberOfHMetrics;

    /// 'hmtx' horizontal metrics
    std::span<std::byte const> hmtxTableBytes;

    /// 'loca' index to location
    std::span<std::byte const> locaTableBytes;

    /// 'maxp' maximum profile
    std::span<std::byte const> maxpTableBytes;
    int numGlyphs;

    /// 'name' naming (not needed)
    std::span<std::byte const> nameTableBytes;

    /// 'post' PostScript (not needed)
    std::span<std::byte const> postTableBytes;

    /// 'OS/2' OS/2 (not needed)
    std::span<std::byte const> os2TableBytes;

    /// 'kern' Kerning tables (optional)
    std::span<std::byte const> kernTableBytes;

public:
    /** Load a true type font.
     * The methods in this class will parse the true-type font at run time.
     * This also means that the bytes passed into this constructor will need to
     * remain available.
     */
    TrueTypeFont(std::span<std::byte const> bytes) :
        file_bytes(bytes)
    {
        parseFontDirectory();
    }

    TrueTypeFont(std::unique_ptr<ResourceView> view) :
        view(std::move(view))
    {
        file_bytes = this->view->bytes();
        parseFontDirectory();
    }

    TrueTypeFont(URL const &url) :
        view(url.loadView())
    {
        file_bytes = this->view->bytes();
        try {
            parseFontDirectory();

        } catch (...) {
            error_info(true).set<url_tag>(url);
            throw;
        }
    }

    TrueTypeFont() = delete;
    TrueTypeFont(TrueTypeFont const &other) = delete;
    TrueTypeFont &operator=(TrueTypeFont const &other) = delete;
    TrueTypeFont(TrueTypeFont &&other) = delete;
    TrueTypeFont &operator=(TrueTypeFont &&other) = delete;
    ~TrueTypeFont() = default;

    /** Get the glyph for a code-point.
    * @return glyph-index, or invalid when not found or error.
    */
    [[nodiscard]] GlyphID find_glyph(char32_t c) const noexcept override;
    
    /** Load a glyph into a path.
     * The glyph is directly loaded from the font file.
     * 
     * @param glyph_id the index of a glyph inside the font.
     * @param path The path constructed by the loader.
     * @return empty on failure, or the glyphID of the metrics to use.
     */
    std::optional<GlyphID> loadGlyph(GlyphID glyph_id, Path &path) const noexcept override;

    /** Load a glyphMetrics into a path.
    * The glyph is directly loaded from the font file.
    * 
    * @param glyph_id the index of a glyph inside the font.
    * @param metrics The metrics constructed by the loader.
    * @param lookahead_glyph_id The next glyph, used for determining kerning.
    * @return 1 on success, 0 on not implemented
    */
    bool loadGlyphMetrics(GlyphID glyph_id, GlyphMetrics &metrics, GlyphID lookahead_glyph_id=GlyphID{}) const noexcept override;

private:
    /** Parses the directory table of the font file.
     * This function is called by the constructor to set up references
     * inside the file for each table.
     */
    void parseFontDirectory();

    /** Parses the head table of the font file.
     * This function is called by parseFontDirectory().
     */
    void parseHeadTable(std::span<std::byte const> headTableBytes);

    void parseHheaTable(std::span<std::byte const> bytes);

    void parseNameTable(std::span<std::byte const> bytes);

    void parseOS2Table(std::span<std::byte const> bytes);

    /** Parse the character map to create unicode_ranges.
     */
    [[nodiscard]] UnicodeRanges parseCharacterMap();


    /** Parses the maxp table of the font file.
    * This function is called by parseFontDirectory().
    */
    void parseMaxpTable(std::span<std::byte const> bytes);

    /** Find the glyph in the loca table.
     * called by loadGlyph()
     */
    bool getGlyphBytes(GlyphID glyph_id, std::span<std::byte const> &bytes) const noexcept;

    /** Update the glyph metrics from the font tables.
     * called by loadGlyph()
     */
    bool updateGlyphMetrics(GlyphID glyph_id, GlyphMetrics &metrics, GlyphID kern_glyph1_id=GlyphID{}, GlyphID kern_glyph2_id=GlyphID{}) const noexcept;

    bool loadSimpleGlyph(std::span<std::byte const> bytes, Path &glyph) const noexcept;

    /** Load a compound glyph.
     * This will call loadGlyph() recursively.
     *
     * \param bytes Bytes inside the glyf table of this specific compound glyph.
     * \param glyph The path to update with points from the subglyphs.
     * \param metricsGlyphIndex The glyph index of the glyph to use for the metrics.
     *                          this value is only updated when the USE_MY_METRICS flag was set.
     */
    bool loadCompoundGlyph(std::span<std::byte const> bytes, Path &glyph, GlyphID &metrics_glyph_id) const noexcept;

    /** Load a compound glyph.
    * This will call loadGlyph() recursively.
    *
    * \param bytes Bytes inside the glyf table of this specific compound glyph.
    * \param metricsGlyphIndex The glyph index of the glyph to use for the metrics.
    *                          this value is only updated when the USE_MY_METRICS flag was set.
    */
    bool loadCompoundGlyphMetrics(std::span<std::byte const> bytes, GlyphID &metrics_glyph_id) const noexcept;


};

}

