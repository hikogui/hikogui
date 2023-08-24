// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "font.hpp"
#include "font_family_id.hpp"
#include "elusive_icon.hpp"
#include "hikogui_icon.hpp"
#include "../unicode/unicode.hpp"
#include "../geometry/module.hpp"
#include "../utility/utility.hpp"
#include "../coroutine/module.hpp"
#include <limits>
#include <array>
#include <new>
#include <atomic>
#include <filesystem>

namespace hi::inline v1 {

/** font_book keeps track of multiple fonts.
 * The font_book is instantiated during application startup
 * and is available through Foundation_globals->font_book.
 *
 *
 */
class font_book {
public:
    struct font_glyph_type {
        hi::font const *font = nullptr;
        hi::glyph_id id = {};

        constexpr font_glyph_type() noexcept = default;
        constexpr font_glyph_type(hi::font const &font, glyph_id id) noexcept : font(std::addressof(font)), id(id) {}

        [[nodiscard]] constexpr friend bool operator==(font_glyph_type const&, font_glyph_type const&) noexcept = default;

        [[nodiscard]] font_metrics const &get_font_metrics() const noexcept
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
        font_glyphs_type(hi::font const &font, lean_vector<glyph_id> ids) noexcept : font(std::addressof(font)), ids(std::move(ids)) {}
        font_glyphs_type(hi::font const &font, glyph_id id) noexcept : font(std::addressof(font)), ids{id} {}

        [[nodiscard]] font_metrics const &get_font_metrics() const noexcept
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

    ~font_book();
    font_book(font_book const&) = delete;
    font_book(font_book&&) = delete;
    font_book& operator=(font_book const&) = delete;
    font_book& operator=(font_book&&) = delete;
    font_book();

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
    font& register_font_file(std::filesystem::path const& path, bool post_process = true);

    /** Register all fonts found in a directory.
     *
     * @see register_font()
     */
    void register_font_directory(std::filesystem::path const& path, bool post_process = true);

    /** Post process font_book
     * Should be called after a set of register_font() calls
     * This calculates font fallbacks.
     */
    void post_process() noexcept;

    /** Find font family id.
     * This function will always return a valid font_family_id by walking the fallback-chain.
     */
    [[nodiscard]] font_family_id find_family(std::string const& family_name) const noexcept;

    /** Register font family id.
     * If the family already exists the existing family_id is returned.
     */
    [[nodiscard]] font_family_id register_family(std::string_view family_name) noexcept;

    /** Find a font closest to the variant.
     * This function will always return a valid font_id.
     *
     * @param family_id a valid family id.
     * @param variant The variant of the font to select.
     * @return a valid font id.
     */
    [[nodiscard]] font const& find_font(font_family_id family_id, font_variant variant) const noexcept;

    /** Find a font closest to the variant.
     * This function will always return a valid font_id.
     *
     * @param family_name A name of a font family, which may be invalid.
     * @param weight The weight of the font to select.
     * @param style If the font to select should be italic or not.
     * @return A pointer to a font, or nullptr when the family was not found.
     */
    [[nodiscard]] font const *find_font(std::string const& family_name, font_variant variant) const noexcept;

    /** Find a glyph using the given code-point.
     * This function will find a glyph matching the grapheme in the selected font, or
     * find the glyph in the fallback font.
     *
     * @param font The font to use to find the grapheme in.
     * @param grapheme The Unicode grapheme to find in the font.
     * @return A list of glyphs which matched the grapheme.
     */
    [[nodiscard]] font_glyphs_type find_glyph(font const& font, grapheme grapheme) const noexcept;

    /** Find a glyph using the given code-point.
     * This function will find a glyph matching the grapheme in the selected font, or
     * find the glyph in the fallback font.
     *
     * @param font The font to use to find the grapheme in.
     * @param grapheme The Unicode grapheme to find in the font.
     * @return A list of glyphs which matched the grapheme.
     */
    [[nodiscard]] font_glyph_type find_glyph(font const& font, char32_t code_point) const noexcept;

private:
    inline static std::unique_ptr<font_book> _global = nullptr;

    /** Table of font_family_ids index using the family-name.
     */
    std::unordered_map<std::string, font_family_id> _family_names;

    /** Different fonts; variants of a family.
     */
    std::vector<std::array<font const *, font_variant::size()>> _font_variants;

    std::vector<std::unique_ptr<font>> _fonts;
    std::vector<hi::font *> _font_ptrs;

    [[nodiscard]] std::vector<hi::font *> make_fallback_chain(font_weight weight, font_style style) noexcept;
};

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
inline font& register_font_file(std::filesystem::path const& path)
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
[[nodiscard]] inline font const& find_font(font_family_id family_id, font_variant variant = font_variant{}) noexcept
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
[[nodiscard]] inline font const *find_font(std::string const& family_name, font_variant variant = font_variant{}) noexcept
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
[[nodiscard]] inline auto find_glyph(font const& font, grapheme grapheme) noexcept
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
[[nodiscard]] inline auto find_glyph(font const& font, char32_t code_point) noexcept
{
    return font_book::global().find_glyph(font, code_point);
}

[[nodiscard]] inline auto find_glyph(elusive_icon rhs) noexcept
{
    hilet *font = find_font("elusiveicons", font_variant{font_weight::medium, font_style::normal});
    hi_assert_not_null(font, "Could not find Elusive icon font");
    return find_glyph(*font, std::to_underlying(rhs));
}

[[nodiscard]] inline auto find_glyph(hikogui_icon rhs) noexcept
{
    hilet *font = find_font("Hikogui Icons", font_variant{font_weight::regular, font_style::normal});
    hi_assert_not_null(font, "Could not find HikoGUI icon font");
    return find_glyph(*font, std::to_underlying(rhs));
}

} // namespace hi::inline v1
