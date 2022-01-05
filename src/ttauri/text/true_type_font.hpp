// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "font.hpp"
#include "../graphic_path.hpp"
#include "../resource_view.hpp"
#include "../URL.hpp"
#include "../counters.hpp"
#include <memory>

namespace tt::inline v1 {

class true_type_font final : public font {
private:
    /** The url to retrieve the view.
     */
    std::optional<URL> url;

    /** The resource view of the font-file.
     *
     * This view may be reset if there is a url available.
     */
    mutable std::unique_ptr<resource_view> view;

    uint16_t OS2_x_height = 0;
    uint16_t OS2_cap_height = 0;

    float unitsPerEm;
    float emScale;
    bool locaTableIsOffset32;

    uint16_t numberOfHMetrics;

    int numGlyphs;

public:
    true_type_font(std::unique_ptr<resource_view> view) : url(), view(std::move(view))
    {
        parse_font_directory();
    }

    true_type_font(URL const &url) : url(url), view(url.loadView())
    {
        ++global_counter<"ttf:map">;
        try {
            parse_font_directory();

            // Clear the view to reclaim resources.
            view = {};
            ++global_counter<"ttf:unmap">;

        } catch (std::exception const &e) {
            throw parse_error("{}: Could not parse font directory.\n{}", to_string(url), e.what());
        }
    }

    true_type_font() = delete;
    true_type_font(true_type_font const &other) = delete;
    true_type_font &operator=(true_type_font const &other) = delete;
    true_type_font(true_type_font &&other) = delete;
    true_type_font &operator=(true_type_font &&other) = delete;
    ~true_type_font() = default;

    [[nodiscard]] bool loaded() const noexcept override
    {
        return static_cast<bool>(view);
    }

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
    std::optional<tt::glyph_id> load_glyph(tt::glyph_id glyph_id, graphic_path &path) const noexcept override;

    /** Load a glyphMetrics into a path.
     * The glyph is directly loaded from the font file.
     *
     * @param glyph_id the index of a glyph inside the font.
     * @param metrics The metrics constructed by the loader.
     * @param lookahead_glyph_id The next glyph, used for determining kerning.
     * @return 1 on success, 0 on not implemented
     */
    bool load_glyph_metrics(tt::glyph_id glyph_id, glyph_metrics &metrics, tt::glyph_id lookahead_glyph_id = tt::glyph_id{})
        const noexcept override;

    [[nodiscard]] vector2 get_kerning(tt::glyph_id current_glyph, tt::glyph_id next_glyph) const noexcept override;

private:
    /** Get the bytes of a table.
     *
     * @return The bytes of a table, or empty if the table does not exist.
     */
    [[nodiscard]] std::span<std::byte const> getTableBytes(char const *table_name) const;

    /** Parses the directory table of the font file.
     *
     * This function is called by the constructor to set up references
     * inside the file for each table.
     */
    void parse_font_directory();

    /** Parses the head table of the font file.
     *
     * This function is called by parse_font_directory().
     */
    void parseHeadTable(std::span<std::byte const> headTableBytes);

    void parseHheaTable(std::span<std::byte const> bytes);

    void parseNameTable(std::span<std::byte const> bytes);

    void parseOS2Table(std::span<std::byte const> bytes);

    /** Parse the character map to create unicode_ranges.
     */
    [[nodiscard]] tt::unicode_mask parseCharacterMap();

    /** Parses the maxp table of the font file.
     *
     * This function is called by parse_font_directory().
     */
    void parseMaxpTable(std::span<std::byte const> bytes);

    /** Find the glyph in the loca table.
     * called by loadGlyph()
     */
    bool getGlyphBytes(tt::glyph_id glyph_id, std::span<std::byte const> &bytes) const noexcept;

    /** Update the glyph metrics from the font tables.
     * called by loadGlyph()
     */
    bool update_glyph_metrics(
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

} // namespace tt::inline v1
