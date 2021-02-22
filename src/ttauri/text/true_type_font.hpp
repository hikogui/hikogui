// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "font.hpp"
#include "../graphic_path.hpp"
#include "../resource_view.hpp"
#include "../URL.hpp"
#include <memory>

namespace tt {

class true_type_font final: public font {
private:
    std::unique_ptr<resource_view> view;

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
    true_type_font(std::span<std::byte const> bytes) :
        file_bytes(bytes)
    {
        parsefontDirectory();
    }

    true_type_font(std::unique_ptr<resource_view> view) :
        view(std::move(view))
    {
        file_bytes = this->view->bytes();
        parsefontDirectory();
    }

    true_type_font(URL const &url) :
        view(url.loadView())
    {
        file_bytes = this->view->bytes();
        try {
            parsefontDirectory();

        } catch (std::exception const &e) {
            throw parse_error("{}: Could not parse font directory.\n{}", url, e.what());
        }
    }

    true_type_font() = delete;
    true_type_font(true_type_font const &other) = delete;
    true_type_font &operator=(true_type_font const &other) = delete;
    true_type_font(true_type_font &&other) = delete;
    true_type_font &operator=(true_type_font &&other) = delete;
    ~true_type_font() = default;

    /** Get the glyph for a code-point.
    * @return glyph-index, or invalid when not found or error.
    */
    [[nodiscard]] tt::glyph_id find_glyph(char32_t c) const noexcept override;
    
    /** Load a glyph into a path.
     * The glyph is directly loaded from the font file.
     * 
     * @param glyph_id the index of a glyph inside the font.
     * @param path The path constructed by the loader.
     * @return empty on failure, or the glyphID of the metrics to use.
     */
    std::optional<tt::glyph_id> loadGlyph(tt::glyph_id glyph_id, graphic_path &path) const noexcept override;

    /** Load a glyphMetrics into a path.
    * The glyph is directly loaded from the font file.
    * 
    * @param glyph_id the index of a glyph inside the font.
    * @param metrics The metrics constructed by the loader.
    * @param lookahead_glyph_id The next glyph, used for determining kerning.
    * @return 1 on success, 0 on not implemented
    */
    bool loadglyph_metrics(tt::glyph_id glyph_id, glyph_metrics &metrics, tt::glyph_id lookahead_glyph_id = tt::glyph_id{})
        const noexcept override;

private:
    /** Parses the directory table of the font file.
     * This function is called by the constructor to set up references
     * inside the file for each table.
     */
    void parsefontDirectory();

    /** Parses the head table of the font file.
     * This function is called by parsefontDirectory().
     */
    void parseHeadTable(std::span<std::byte const> headTableBytes);

    void parseHheaTable(std::span<std::byte const> bytes);

    void parseNameTable(std::span<std::byte const> bytes);

    void parseOS2Table(std::span<std::byte const> bytes);

    /** Parse the character map to create unicode_ranges.
     */
    [[nodiscard]] unicode_ranges parseCharacterMap();


    /** Parses the maxp table of the font file.
    * This function is called by parsefontDirectory().
    */
    void parseMaxpTable(std::span<std::byte const> bytes);

    /** Find the glyph in the loca table.
     * called by loadGlyph()
     */
    bool getGlyphBytes(tt::glyph_id glyph_id, std::span<std::byte const> &bytes) const noexcept;

    /** Update the glyph metrics from the font tables.
     * called by loadGlyph()
     */
    bool updateglyph_metrics(
        tt::glyph_id glyph_id,
        glyph_metrics &metrics,
        tt::glyph_id kern_glyph1_id = tt::glyph_id{},
        tt::glyph_id kern_glyph2_id = tt::glyph_id{}) const noexcept;

    bool loadSimpleGlyph(std::span<std::byte const> bytes, graphic_path &glyph) const noexcept;

    /** Load a compound glyph.
     * This will call loadGlyph() recursively.
     *
     * \param bytes Bytes inside the glyf table of this specific compound glyph.
     * \param glyph The path to update with points from the subglyphs.
     * \param metricsGlyphIndex The glyph index of the glyph to use for the metrics.
     *                          this value is only updated when the USE_MY_METRICS flag was set.
     */
    bool loadCompoundGlyph(std::span<std::byte const> bytes, graphic_path &glyph, tt::glyph_id &metrics_glyph_id) const noexcept;

    /** Load a compound glyph.
    * This will call loadGlyph() recursively.
    *
    * \param bytes Bytes inside the glyf table of this specific compound glyph.
    * \param metricsGlyphIndex The glyph index of the glyph to use for the metrics.
    *                          this value is only updated when the USE_MY_METRICS flag was set.
    */
    bool loadCompoundglyph_metrics(std::span<std::byte const> bytes, tt::glyph_id &metrics_glyph_id) const noexcept;


};

}

