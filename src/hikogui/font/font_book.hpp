// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "font_font.hpp"
#include "font_family_id.hpp"
#include "true_type_font.hpp"
#include "elusive_icon.hpp"
#include "hikogui_icon.hpp"
#include "../unicode/unicode.hpp"
#include "../geometry/geometry.hpp"
#include "../utility/utility.hpp"
#include "../coroutine/coroutine.hpp"
#include "../path/path.hpp"
#include <limits>
#include <array>
#include <new>
#include <atomic>
#include <filesystem>

hi_export_module(hikogui.font.font_book);

hi_export namespace hi::inline v1 {

/** font_book keeps track of multiple fonts.
 * The font_book is instantiated during application startup
 * and is available through Foundation_globals->font_book.
 *
 *
 */
hi_export class font_book {
public:
    struct font_glyph_type {
        hi::font const *font = nullptr;
        hi::glyph_id id = {};

        constexpr font_glyph_type() noexcept = default;
        constexpr font_glyph_type(hi::font const& font, glyph_id id) noexcept : font(std::addressof(font)), id(id) {}

        [[nodiscard]] constexpr friend bool operator==(font_glyph_type const&, font_glyph_type const&) noexcept = default;

        [[nodiscard]] font_metrics const& get_font_metrics() const noexcept
        {
            hi_axiom_not_null(font);
            return font->metrics;
        }

        [[nodiscard]] glyph_metrics get_metrics() const noexcept
        {
            hi_axiom_not_null(font);
            return font->get_metrics(id);
        }

        [[nodiscard]] aarectangle get_bounding_rectangle() const noexcept
        {
            return get_metrics().bounding_rectangle;
        }
    };

    struct font_glyphs_type {
        hi::font const *font = nullptr;
        lean_vector<glyph_id> ids = {};

        constexpr font_glyphs_type() noexcept = default;
        font_glyphs_type(hi::font const& font, lean_vector<glyph_id> ids) noexcept :
            font(std::addressof(font)), ids(std::move(ids))
        {
        }
        font_glyphs_type(hi::font const& font, glyph_id id) noexcept : font(std::addressof(font)), ids{id} {}

        [[nodiscard]] font_metrics const& get_font_metrics() const noexcept
        {
            hi_axiom_not_null(font);
            return font->metrics;
        }

        [[nodiscard]] glyph_metrics get_starter_metrics() const noexcept
        {
            hi_axiom(not ids.empty());
            hi_axiom_not_null(font);
            return font->get_metrics(ids.front());
        }
    };

    static font_book& global() noexcept;

    ~font_book() = default;
    font_book(font_book const&) = delete;
    font_book(font_book&&) = delete;
    font_book& operator=(font_book const&) = delete;
    font_book& operator=(font_book&&) = delete;
    font_book() = default;

    /** Register a font.
     * Duplicate registrations will be ignored.
     *
     * When a font file is registered the file will be temporarily opened to read and cache a set of properties:
     *  - The English font Family from the 'name' table.
     *  - The weight, width, slant & design-size from the 'fdsc' table.
     *  - The character map 'cmap' table.
     *
     * @param path Location of font.
     * @param post_process Calculate font fallback
     */
    font& register_font_file(std::filesystem::path const& path, bool post_process = true)
    {
        auto font = std::make_unique<true_type_font>(path);
        auto font_ptr = font.get();

        hi_log_info("Parsed font {}: {}", path.string(), to_string(*font));

        hilet font_family_id = register_family(font->family_name);
        _font_variants[*font_family_id][font->font_variant()] = font_ptr;

        _fonts.emplace_back(std::move(font));
        _font_ptrs.push_back(font_ptr);

        if (post_process) {
            this->post_process();
        }

        return *font_ptr;
    }

    /** Register all fonts found in a directory.
     *
     * @see register_font()
     */
    void register_font_directory(std::filesystem::path const& path, bool post_process = true)
    {
        hilet font_directory_glob = path / "**" / "*.ttf";
        for (hilet& font_path : glob(font_directory_glob)) {
            hilet t = trace<"font_scan">{};

            try {
                register_font_file(font_path, false);

            } catch (std::exception const& e) {
                hi_log_error("Failed parsing font at {}: \"{}\"", font_path.string(), e.what());
            }
        }

        if (post_process) {
            this->post_process();
        }
    }

    /** Post process font_book
     * Should be called after a set of register_font() calls
     * This calculates font fallbacks.
     */
    void post_process() noexcept
    {
        // Sort the list of fonts based on the amount of unicode code points it supports.
        std::sort(begin(_font_ptrs), end(_font_ptrs), [](hilet& lhs, hilet& rhs) {
            return lhs->char_map.count() > rhs->char_map.count();
        });

        hilet regular_fallback_chain = make_fallback_chain(font_weight::regular, font_style::normal);
        hilet bold_fallback_chain = make_fallback_chain(font_weight::bold, font_style::normal);
        hilet italic_fallback_chain = make_fallback_chain(font_weight::regular, font_style::italic);

        hi_log_info(
            "Post processing fonts number={}, regular-fallback={}, bold-fallback={}, italic-fallback={}",
            size(_fonts),
            size(regular_fallback_chain),
            size(bold_fallback_chain),
            size(italic_fallback_chain));

        // For each font, find fallback list.
        for (hilet& font : _font_ptrs) {
            auto fallback_chain = std::vector<hi::font *>{};

            // Put the fonts from the same family, italic and weight first.
            for (hilet& fallback : _font_ptrs) {
                // clang-format off
            if (
                (fallback != font) and
                (fallback->family_name == font->family_name) and
                (fallback->style == font->style) and
                almost_equal(fallback->weight, font->weight)
            ) {
                fallback_chain.push_back(fallback);
            }
                // clang-format on
            }

            if (almost_equal(font->weight, font_weight::bold)) {
                std::copy(begin(bold_fallback_chain), end(bold_fallback_chain), std::back_inserter(fallback_chain));
            } else if (font->style == font_style::italic) {
                std::copy(begin(italic_fallback_chain), end(italic_fallback_chain), std::back_inserter(fallback_chain));
            } else {
                std::copy(begin(regular_fallback_chain), end(regular_fallback_chain), std::back_inserter(fallback_chain));
            }

            font->fallback_chain = std::move(fallback_chain);
        }
    }

    /** Find font family id.
     * This function will always return a valid font_family_id by walking the fallback-chain.
     */
    [[nodiscard]] font_family_id find_family(std::string const& family_name) const noexcept
    {
        auto it = _family_names.find(to_lower(family_name));
        if (it == _family_names.end()) {
            return std::nullopt;
        } else {
            return it->second;
        }
    }

    /** Register font family id.
     * If the family already exists the existing family_id is returned.
     */
    [[nodiscard]] font_family_id register_family(std::string_view family_name) noexcept
    {
        auto name = to_lower(family_name);

        auto it = _family_names.find(name);
        if (it == _family_names.end()) {
            hilet family_id = font_family_id(_font_variants.size());
            _font_variants.emplace_back();
            _family_names[name] = family_id;
            return family_id;
        } else {
            return it->second;
        }
    }

    /** Find a font closest to the variant.
     * This function will always return a valid font_id.
     *
     * @param family_id a valid family id.
     * @param variant The variant of the font to select.
     * @return a valid font id.
     */
    [[nodiscard]] font const& find_font(font_family_id family_id, font_variant variant) const noexcept
    {
        hi_assert(family_id);
        hi_assert_bounds(*family_id, _font_variants);

        hilet& variants = _font_variants[*family_id];
        for (auto alternative_variant : alternatives(variant)) {
            if (auto font = variants[alternative_variant]) {
                return *font;
            }
        }

        // If a family exists, there must be at least one font variant available.
        hi_no_default();
    }

    /** Find a font closest to the variant.
     * This function will always return a valid font_id.
     *
     * @param family_name A name of a font family, which may be invalid.
     * @param weight The weight of the font to select.
     * @param style If the font to select should be italic or not.
     * @return A pointer to a font, or nullptr when the family was not found.
     */
    [[nodiscard]] font const *find_font(std::string const& family_name, font_variant variant) const noexcept
    {
        if (hilet family_id = find_family(family_name)) {
            return &find_font(family_id, variant);
        } else {
            return nullptr;
        }
    }

    /** Find a glyph using the given code-point.
     * This function will find a glyph matching the grapheme in the selected font, or
     * find the glyph in the fallback font.
     *
     * @param font The font to use to find the grapheme in.
     * @param grapheme The Unicode grapheme to find in the font.
     * @return A list of glyphs which matched the grapheme.
     */
    [[nodiscard]] font_glyphs_type find_glyph(font const& font, hi::grapheme grapheme) const noexcept
    {
        // First try the selected font.
        if (hilet glyph_ids = font.find_glyph(grapheme); not glyph_ids.empty()) {
            return {font, std::move(glyph_ids)};
        }

        // Scan fonts which are fallback to this.
        for (hilet fallback : font.fallback_chain) {
            hi_axiom_not_null(fallback);
            if (hilet glyph_ids = fallback->find_glyph(grapheme); not glyph_ids.empty()) {
                return {*fallback, std::move(glyph_ids)};
            }
        }

        // If all everything has failed, use the tofu block of the original font.
        return {font, {glyph_id{0}}};
    }

    /** Find a glyph using the given code-point.
     * This function will find a glyph matching the grapheme in the selected font, or
     * find the glyph in the fallback font.
     *
     * @param font The font to use to find the grapheme in.
     * @param grapheme The Unicode grapheme to find in the font.
     * @return A list of glyphs which matched the grapheme.
     */
    [[nodiscard]] font_glyph_type find_glyph(font const& font, char32_t code_point) const noexcept
    {
        // First try the selected font.
        if (hilet glyph_id = font.find_glyph(code_point)) {
            return {font, glyph_id};
        }

        // Scan fonts which are fallback to this.
        for (hilet fallback : font.fallback_chain) {
            hi_axiom_not_null(fallback);
            if (hilet glyph_id = fallback->find_glyph(code_point)) {
                return {*fallback, glyph_id};
            }
        }

        // If all everything has failed, use the tofu block of the original font.
        return {font, glyph_id{0}};
    }

private:
    /** Table of font_family_ids index using the family-name.
     */
    std::unordered_map<std::string, font_family_id> _family_names;

    /** Different fonts; variants of a family.
     */
    std::vector<std::array<font const *, font_variant::size()>> _font_variants;

    std::vector<std::unique_ptr<font>> _fonts;
    std::vector<hi::font *> _font_ptrs;

    [[nodiscard]] std::vector<hi::font *> make_fallback_chain(font_weight weight, font_style style) noexcept
    {
        auto r = _font_ptrs;

        std::stable_partition(begin(r), end(r), [weight, style](hilet& item) {
            return (item->style == style) and almost_equal(item->weight, weight);
        });

        auto char_mask = std::bitset<0x11'0000>{};
        for (auto& font : r) {
            if (font->char_map.update_mask(char_mask) == 0) {
                // This font did not add any code points.
                font = nullptr;
            }
        }

        std::erase(r, nullptr);
        return r;
    }
};

namespace detail {
hi_inline std::unique_ptr<font_book> font_book_global = nullptr;
}

hi_inline font_book& font_book::global() noexcept
{
    if (not detail::font_book_global) {
        detail::font_book_global = std::make_unique<font_book>();
    }
    return *detail::font_book_global;
}

/** Register a font.
 * Duplicate registrations will be ignored.
 *
 * When a font file is registered the file will be temporarily opened to read and cache a set of properties:
 *  - The English font Family from the 'name' table.
 *  - The weight, width, slant & design-size from the 'fdsc' table.
 *  - The character map 'cmap' table.
 *
 * @param path Location of font.
 */
hi_export hi_inline font& register_font_file(std::filesystem::path const& path)
{
    return font_book::global().register_font_file(path);
}

hi_export hi_inline void register_font_directory(std::filesystem::path const& path)
{
    return font_book::global().register_font_directory(path);
}

hi_export template<typename Range>
hi_inline void register_font_directories(Range&& range) noexcept
{
    for (auto const& path : range) {
        font_book::global().register_font_directory(path, false);
    }
    font_book::global().post_process();
}

/** Find font family id.
 * This function will always return a valid font_family_id by walking the fallback-chain.
 */
hi_export [[nodiscard]] hi_inline font_family_id find_font_family(std::string const& family_name) noexcept
{
    return font_book::global().find_family(family_name);
}

/** Find a font closest to the variant.
 * This function will always return a valid font_id.
 *
 * @param family_id a valid family id.
 * @param variant The variant of the font to select.
 * @return a valid font id.
 */
hi_export [[nodiscard]] hi_inline font const& find_font(font_family_id family_id, font_variant variant = font_variant{}) noexcept
{
    return font_book::global().find_font(family_id, variant);
}

/** Find a font closest to the variant.
 * This function will always return a valid font_id.
 *
 * @param family_name a font family name.
 * @param variant The variant of the font to select.
 * @return A pointer to the loaded font.
 */
hi_export [[nodiscard]] hi_inline font const *find_font(std::string const& family_name, font_variant variant = font_variant{}) noexcept
{
    return font_book::global().find_font(family_name, variant);
}

/** Find a glyph using the given code-point.
 * This function will find a glyph matching the grapheme in the selected font, or
 * find the glyph in the fallback font.
 *
 * @param font The font to use to find the grapheme in.
 * @param grapheme The Unicode grapheme to find in the font.
 * @return A list of glyphs which matched the grapheme.
 */
hi_export [[nodiscard]] hi_inline auto find_glyph(font const& font, grapheme grapheme) noexcept
{
    return font_book::global().find_glyph(font, grapheme);
}

/** Find a glyph using the given code-point.
 * This function will find a glyph matching the grapheme in the selected font, or
 * find the glyph in the fallback font.
 *
 * @param font The font to use to find the grapheme in.
 * @param grapheme The Unicode grapheme to find in the font.
 * @return A list of glyphs which matched the grapheme.
 */
hi_export [[nodiscard]] hi_inline auto find_glyph(font const& font, char32_t code_point) noexcept
{
    return font_book::global().find_glyph(font, code_point);
}

hi_export [[nodiscard]] hi_inline auto find_glyph(elusive_icon rhs) noexcept
{
    hilet *font = find_font("elusiveicons", font_variant{font_weight::medium, font_style::normal});
    hi_assert_not_null(font, "Could not find Elusive icon font");
    return find_glyph(*font, std::to_underlying(rhs));
}

hi_export [[nodiscard]] hi_inline auto find_glyph(hikogui_icon rhs) noexcept
{
    hilet *font = find_font("Hikogui Icons", font_variant{font_weight::regular, font_style::normal});
    hi_assert_not_null(font, "Could not find HikoGUI icon font");
    return find_glyph(*font, std::to_underlying(rhs));
}

} // namespace hi::inline v1
