// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "font.hpp"
#include "otype_sfnt.hpp"
#include "otype_kern.hpp"
#include "font_char_map.hpp"
#include "../file/file_view.hpp"
#include "../graphic_path.hpp"
#include "../counters.hpp"
#include "../utility/module.hpp"
#include <memory>
#include <filesystem>

namespace hi::inline v1 {

class true_type_font final : public font {
public:
    true_type_font(std::filesystem::path const& path) : _path(path), _view(file_view{path})
    {
        ++global_counter<"ttf:map">;
        try {
            _bytes = as_span<std::byte const>(_view);
            parse_font_directory(_bytes);

            // Clear the view to reclaim resources.
            _view = {};
            _bytes = {};
            ++global_counter<"ttf:unmap">;

        } catch (std::exception const &e) {
            throw parse_error(std::format("{}: Could not parse font directory.\n{}", path.string(), e.what()));
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
        return to_bool(_view);
    }

    /** Get the glyph for a code-point.
     * @return glyph-index, or invalid when not found or error.
     */
    [[nodiscard]] hi::glyph_id find_glyph(char32_t c) const override;

    /** Load a glyph into a path.
     * The glyph is directly loaded from the font file.
     *
     * @param glyph_id the index of a glyph inside the font.
     * @param path The path constructed by the loader.
     * @return empty on failure, or the glyphID of the metrics to use.
     */
    std::optional<hi::glyph_id> load_glyph(hi::glyph_id glyph_id, graphic_path &path) const override;

    /** Load a glyphMetrics into a path.
     * The glyph is directly loaded from the font file.
     *
     * @param glyph_id the index of a glyph inside the font.
     * @param metrics The metrics constructed by the loader.
     * @param lookahead_glyph_id The next glyph, used for determining kerning.
     * @return 1 on success, 0 on not implemented
     */
    bool load_glyph_metrics(hi::glyph_id glyph_id, glyph_metrics &metrics, hi::glyph_id lookahead_glyph_id = hi::glyph_id{})
        const override;

    [[nodiscard]] vector2 get_kerning(hi::glyph_id current_glyph, hi::glyph_id next_glyph) const override
    {
        load_view();
        return otype_kern_find(_kern_table_bytes, current_glyph, next_glyph, _em_scale);
    }

    void substitution_and_kerning(iso_639 language, iso_15924 script, std::vector<substitution_and_kerning_type> &word)
        const override
    {
    }

private:
    /** The url to retrieve the view.
     */
    std::filesystem::path _path;

    /** The resource view of the font-file.
     *
     * This view may be reset if there is a path available.
     */
    mutable file_view _view;

    float OS2_x_height = 0;
    float OS2_cap_height = 0;

    float _em_scale;

    uint16_t numberOfHMetrics;

    int num_glyphs;
    mutable std::span<std::byte const> _bytes;
    mutable std::span<std::byte const> _loca_table_bytes;
    mutable std::span<std::byte const> _glyf_table_bytes;
    mutable std::span<std::byte const> _hmtx_table_bytes;
    mutable std::span<std::byte const> _kern_table_bytes;
    mutable std::span<std::byte const> _GSUB_table_bytes;
    bool _loca_is_offset32;
    font_char_map _char_map;

    void cache_tables(std::span<std::byte const> bytes) const
    {
        _loca_table_bytes = otype_sfnt_search<"loca">(bytes);
        _glyf_table_bytes = otype_sfnt_search<"glyf">(bytes);
        _hmtx_table_bytes = otype_sfnt_search<"hmtx">(bytes);

        // Optional tables.
        _kern_table_bytes = otype_sfnt_search<"kern">(bytes);
        _GSUB_table_bytes = otype_sfnt_search<"GSUB">(bytes);
    }

    void load_view() const noexcept
    {
        if (_view) {
            [[likely]] return;
        }

        _view = file_view{_path};
        _bytes = as_span<std::byte const>(_view);
        ++global_counter<"ttf:map">;
        cache_tables(_bytes);
    }

    /** Parses the directory table of the font file.
     *
     * This function is called by the constructor to set up references
     * inside the file for each table.
     */
    void parse_font_directory(std::span<std::byte const> bytes);

    /** Parse the character map to create unicode_ranges.
     */
    [[nodiscard]] hi::unicode_mask parse_cmap_table_mask() const;

    /** Update the glyph metrics from the font tables.
     * called by loadGlyph()
     */
    bool update_glyph_metrics(
        hi::glyph_id glyph_id,
        glyph_metrics &metrics,
        hi::glyph_id kern_glyph1_id = hi::glyph_id{},
        hi::glyph_id kern_glyph2_id = hi::glyph_id{}) const;

    bool load_simple_glyph(std::span<std::byte const> bytes, graphic_path &glyph) const;

    /** Load a compound glyph.
     * This will call loadGlyph() recursively.
     *
     * \param bytes Bytes inside the glyf table of this specific compound glyph.
     * \param glyph The path to update with points from the subglyphs.
     * \param metricsGlyphIndex The glyph index of the glyph to use for the metrics.
     *                          this value is only updated when the USE_MY_METRICS flag was set.
     */
    bool
    load_compound_glyph(std::span<std::byte const> bytes, graphic_path &glyph, hi::glyph_id &metrics_glyph_id) const;

    /** Load a compound glyph.
     * This will call loadGlyph() recursively.
     *
     * \param bytes Bytes inside the glyf table of this specific compound glyph.
     * \param metricsGlyphIndex The glyph index of the glyph to use for the metrics.
     *                          this value is only updated when the USE_MY_METRICS flag was set.
     */
    bool load_compound_glyph_metrics(std::span<std::byte const> bytes, hi::glyph_id &metrics_glyph_id) const;

    /** Get the index of the glyph from the coverage table.
     *
     * @param bytes The bytes of the coverage table.
     * @param glyph The glyph to search.
     * @return Coverage-index of the glyph when found, -1 if not found, -2 on error.
     */
    [[nodiscard]] std::ptrdiff_t get_coverage_index(std::span<std::byte const> bytes, hi::glyph_id glyph);
};

} // namespace hi::inline v1
