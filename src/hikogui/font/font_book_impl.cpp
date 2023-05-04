// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "font_book.hpp"
#include "true_type_font.hpp"
#include "../file/glob.hpp"
#include "../trace.hpp"
#include "../ranges.hpp"
#include "../log.hpp"
#include <ranges>
#include <vector>

namespace hi::inline v1 {

font_book& font_book::global() noexcept
{
    if (not _global) {
        _global = std::make_unique<font_book>();
    }
    return *_global;
}

font_book::~font_book() {}

font_book::font_book() {}

font& font_book::register_font_file(std::filesystem::path const& path, bool post_process)
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

void font_book::register_font_directory(std::filesystem::path const& path, bool post_process)
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

[[nodiscard]] std::vector<hi::font *> font_book::make_fallback_chain(font_weight weight, font_style style) noexcept
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

void font_book::post_process() noexcept
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

[[nodiscard]] font_family_id font_book::register_family(std::string_view family_name) noexcept
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

[[nodiscard]] font_family_id font_book::find_family(std::string const& family_name) const noexcept
{
    auto it = _family_names.find(to_lower(family_name));
    if (it == _family_names.end()) {
        return std::nullopt;
    } else {
        return it->second;
    }
}

[[nodiscard]] font const& font_book::find_font(font_family_id family_id, font_variant variant) const noexcept
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

[[nodiscard]] font const *font_book::find_font(std::string const& family_name, font_variant variant) const noexcept
{
    if (hilet family_id = find_family(family_name)) {
        return &find_font(family_id, variant);
    } else {
        return nullptr;
    }
}

[[nodiscard]] font_book::font_glyphs_type font_book::find_glyph(hi::font const& font, grapheme g) const noexcept
{
    // First try the selected font.
    if (hilet glyph_ids = font.find_glyph(g); not glyph_ids.empty()) {
        return {&font, std::move(glyph_ids)};
    }

    // Scan fonts which are fallback to this.
    for (hilet fallback : font.fallback_chain) {
        hi_axiom_not_null(fallback);
        if (hilet glyph_ids = fallback->find_glyph(g); not glyph_ids.empty()) {
            return {fallback, std::move(glyph_ids)};
        }
    }

    // If all everything has failed, use the tofu block of the original font.
    return {&font, {glyph_id{0}}};
}

[[nodiscard]] font_book::font_glyph_type font_book::find_glyph(hi::font const& font, char32_t code_point) const noexcept
{
    // First try the selected font.
    if (hilet glyph_id = font.find_glyph(code_point)) {
        return {&font, glyph_id};
    }

    // Scan fonts which are fallback to this.
    for (hilet fallback : font.fallback_chain) {
        hi_axiom_not_null(fallback);
        if (hilet glyph_id = fallback->find_glyph(code_point)) {
            return {fallback, glyph_id};
        }
    }

    // If all everything has failed, use the tofu block of the original font.
    return {&font, glyph_id{0}};
}

} // namespace hi::inline v1
