// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "font_font.hpp"
#include "font_id.hpp"
#include "font_glyph_ids.hpp"
#include "font_family_id.hpp"
#include "true_type_font.hpp"
#include "elusive_icon.hpp"
#include "hikogui_icon.hpp"
#include "../unicode/unicode.hpp"
#include "../geometry/geometry.hpp"
#include "../utility/utility.hpp"
#include "../path/path.hpp"
#include <gsl/gsl>
#include <limits>
#include <array>
#include <new>
#include <atomic>
#include <filesystem>

hi_export_module(hikogui.font : font_book);

hi_export namespace hi::inline v1 {

/** font_book keeps track of multiple fonts.
 * The font_book is instantiated during application startup
 * and is available through Foundation_globals->font_book.
 *
 *
 */
class font_book {
public:
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
    font_id register_font_file(std::filesystem::path const& path, bool post_process = true)
    {
        if (_fonts.size() >= font_id::empty_value) {
            throw std::overflow_error("Too many fonts registered");
        }

        auto const font_id = hi::font_id{gsl::narrow_cast<font_id::value_type>(_fonts.size())};
        auto const &font = *_fonts.emplace_back(std::make_unique<true_type_font>(path));
        _fallback_chain.push_back(font_id);

        hi_log_info("Parsed font id={} {}: {}", *font_id, path.string(), to_string(font));

        auto const font_family_id = register_family(font.family_name);
        _font_variants[*font_family_id][font.font_variant()] = font_id;

        if (post_process) {
            this->post_process();
        }

        return font_id;
    }

    /** Register all fonts found in a directory.
     *
     * @see register_font()
     */
    void register_font_directory(std::filesystem::path const& path, bool post_process = true)
    {
        auto const font_directory_glob = path / "**" / "*.ttf";
        for (auto const& font_path : glob(font_directory_glob)) {
            auto const t = trace<"font_scan">{};

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
        std::sort(begin(_fallback_chain), end(_fallback_chain), [](auto const& lhs, auto const& rhs) {
            return lhs->char_map.count() > rhs->char_map.count();
        });

        auto const regular_fallback_chain = make_fallback_chain(font_weight::regular, font_style::normal);
        auto const bold_fallback_chain = make_fallback_chain(font_weight::bold, font_style::normal);
        auto const italic_fallback_chain = make_fallback_chain(font_weight::regular, font_style::italic);

        hi_log_info(
            "Post processing fonts number={}, regular-fallback={}, bold-fallback={}, italic-fallback={}",
            size(_fonts),
            size(regular_fallback_chain),
            size(bold_fallback_chain),
            size(italic_fallback_chain));

        // For each font, find fallback list.
        for (auto const& font : _fallback_chain) {
            auto fallback_chain = std::vector<hi::font_id>{};

            // Put the fonts from the same family, italic and weight first.
            for (auto const& fallback : _fallback_chain) {
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
    [[nodiscard]] font_family_id register_family(std::string_view family_name)
    {
        auto name = to_lower(family_name);

        auto it = _family_names.find(name);
        if (it == _family_names.end()) {
            if (_font_variants.size() >= font_family_id::empty_value) {
                throw std::overflow_error("Too many font-family-ids registered");
            }

            auto const family_id = font_family_id{gsl::narrow_cast<font_family_id::value_type>(_font_variants.size())};
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
    [[nodiscard]] font_id find_font(font_family_id family_id, font_variant variant) const noexcept
    {
        hi_assert(family_id);
        hi_assert_bounds(*family_id, _font_variants);

        auto const& variants = _font_variants[*family_id];
        for (auto i : alternatives(variant)) {
            if (auto id = variants[i]) {
                return id;
            }
        }

        // If a family exists, there must be at least one font variant available.
        hi_no_default();
    }

    [[nodiscard]] font &get_font(font_id id) const
    {
        return *_fonts.at(*id);
    }

    /** Find a combination of glyphs matching the given grapheme.
     * This function will find a combination of glyphs matching the grapheme
     * in the selected font, or find the glyphs in the fallback font.
     *
     * @param font The font to use to find the grapheme in.
     * @param grapheme The Unicode grapheme to find in the font.
     * @return A list of glyphs which matched the grapheme.
     */
    [[nodiscard]] font_glyph_ids find_glyph(font_id font, hi::grapheme grapheme) const noexcept
    {
        // First try the selected font.
        if (auto const glyph_ids = font->find_glyph(grapheme); not glyph_ids.empty()) {
            return {font, std::move(glyph_ids)};
        }

        // Scan fonts which are fallback to this.
        for (auto const fallback : font->fallback_chain) {
            hi_axiom(not fallback.empty());
            if (auto const glyph_ids = fallback->find_glyph(grapheme); not glyph_ids.empty()) {
                return {*fallback, std::move(glyph_ids)};
            }
        }

        // If all everything has failed, use the tofu block of the original font.
        return {font, {glyph_id{0}}};
    }

private:
    /** Table of font_family_ids index using the family-name.
     */
    std::unordered_map<std::string, font_family_id> _family_names;

    /** Different fonts; variants of a family.
     */
    std::vector<std::array<font_id, font_variant::size()>> _font_variants;

    std::vector<std::unique_ptr<font>> _fonts;
    std::vector<hi::font_id> _fallback_chain;

    [[nodiscard]] std::vector<hi::font_id> make_fallback_chain(font_weight weight, font_style style) noexcept
    {
        auto r = _fallback_chain;

        std::stable_partition(begin(r), end(r), [weight, style](auto const& item) {
            return (item->style == style) and almost_equal(item->weight, weight);
        });

        auto char_mask = std::bitset<0x11'0000>{};
        for (auto& font : r) {
            if (font->char_map.update_mask(char_mask) == 0) {
                // This font did not add any code points.
                font = std::nullopt;
            }
        }

        std::erase(r, std::nullopt);
        return r;
    }
};

namespace detail {
inline std::unique_ptr<font_book> font_book_global = nullptr;
}

inline font_book& font_book::global() noexcept
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
inline font_id register_font_file(std::filesystem::path const& path)
{
    return font_book::global().register_font_file(path);
}

inline void register_font_directory(std::filesystem::path const& path)
{
    return font_book::global().register_font_directory(path);
}

template<typename Range>
inline void register_font_directories(Range&& range) noexcept
{
    for (auto const& path : range) {
        font_book::global().register_font_directory(path, false);
    }
    font_book::global().post_process();
}

/** Find font family id.
 * This function will always return a valid font_family_id by walking the fallback-chain.
 */
[[nodiscard]] inline font_family_id find_font_family(std::string const& family_name) noexcept
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
[[nodiscard]] inline font_id find_font(font_family_id family_id, font_variant variant = font_variant{}) noexcept
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
[[nodiscard]] inline font_id find_font(std::string const& family_name, font_variant variant = font_variant{}) noexcept
{
    if (auto family_id = find_font_family(family_name)) {
        return find_font(family_id, variant);
    } else {
        return font_id{};
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
[[nodiscard]] inline font_glyph_ids find_glyph(font_id font, grapheme grapheme) noexcept
{
    return font_book::global().find_glyph(font, grapheme);
}

[[nodiscard]] inline font_glyph_ids find_glyph(elusive_icon rhs) noexcept
{
    auto const id = find_font("elusiveicons", font_variant{font_weight::medium, font_style::normal});
    hi_assert(not id.empty(), "Could not find Elusive icon font");
    return find_glyph(id, std::to_underlying(rhs));
}

[[nodiscard]] inline font_glyph_ids find_glyph(hikogui_icon rhs) noexcept
{
    auto const id = find_font("Hikogui Icons", font_variant{font_weight::regular, font_style::normal});
    hi_assert(not id.empty(), "Could not find HikoGUI icon font");
    return find_glyph(id, std::to_underlying(rhs));
}

[[nodiscard]] inline font &get_font(font_id id)
{
    return font_book::global().get_font(id);
}

[[nodiscard]] inline font *font_id::operator->() const
{
    return std::addressof(get_font(*this));
}

} // namespace hi::inline v1
