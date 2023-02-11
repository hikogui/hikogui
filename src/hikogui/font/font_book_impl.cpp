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

font_book::font_book()
{
    create_family_name_fallback_chain();
}

void font_book::create_family_name_fallback_chain() noexcept
{
    _family_name_fallback_chain["fallback"] = "sans-serif";

    // Serif web-fonts
    _family_name_fallback_chain["serif"] = "times new roman";
    _family_name_fallback_chain["times new roman"] = "times";
    _family_name_fallback_chain["times"] = "noto serif";
    _family_name_fallback_chain["noto serif"] = "noto";

    _family_name_fallback_chain["georgia"] = "serif";

    _family_name_fallback_chain["palatino"] = "palatino linotype";
    _family_name_fallback_chain["palatino linotype"] = "book antiqua";
    _family_name_fallback_chain["book antiqua"] = "serif";

    // Sans-serif web-fonts
    _family_name_fallback_chain["sans-serif"] = "arial";
    _family_name_fallback_chain["arial"] = "helvetica";
    _family_name_fallback_chain["helvetica"] = "noto sans";

    _family_name_fallback_chain["gadget"] = "sans-sarif";

    _family_name_fallback_chain["comic sans"] = "comic sans ms";
    _family_name_fallback_chain["comic sans ms"] = "cursive";
    _family_name_fallback_chain["cursive"] = "sans-serif";

    _family_name_fallback_chain["impact"] = "charcoal";
    _family_name_fallback_chain["charcoal"] = "sans-serif";

    _family_name_fallback_chain["lucida"] = "lucida sans";
    _family_name_fallback_chain["lucida sans"] = "lucida sans unicode";
    _family_name_fallback_chain["lucida sans unicode"] = "lucida grande";
    _family_name_fallback_chain["lucida grande"] = "sans-serif";

    _family_name_fallback_chain["verdana"] = "geneva";
    _family_name_fallback_chain["tahoma"] = "geneva";
    _family_name_fallback_chain["geneva"] = "sans-serif";

    _family_name_fallback_chain["trebuchet"] = "trebuchet ms";
    _family_name_fallback_chain["trebuchet ms"] = "helvetica";
    _family_name_fallback_chain["helvetic"] = "sans-serif";

    // Monospace web-fonts.
    _family_name_fallback_chain["monospace"] = "courier";
    _family_name_fallback_chain["courier"] = "courier new";

    _family_name_fallback_chain["consolas"] = "lucida console";
    _family_name_fallback_chain["lucida console"] = "monaco";
    _family_name_fallback_chain["monaco"] = "andale mono";
    _family_name_fallback_chain["andale mono"] = "monospace";
}

font& font_book::register_font(std::filesystem::path const& path, bool post_process)
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
            register_font(font_path, false);

        } catch (std::exception const& e) {
            hi_log_error("Failed parsing font at {}: \"{}\"", font_path.string(), e.what());
        }
    }

    if (post_process) {
        this->post_process();
    }
}

[[nodiscard]] std::vector<hi::font *> font_book::make_fallback_chain(font_weight weight, bool italic) noexcept
{
    auto r = _font_ptrs;

    std::stable_partition(begin(r), end(r), [weight, italic](hilet& item) {
        return (item->italic == italic) and almost_equal(item->weight, weight);
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
    // Reset caches and fallback chains.
    _glyph_cache.clear();
    _family_name_cache = _family_names;

    // Sort the list of fonts based on the amount of unicode code points it supports.
    std::sort(begin(_font_ptrs), end(_font_ptrs), [](hilet& lhs, hilet& rhs) {
        return lhs->char_map.count() > rhs->char_map.count();
    });

    hilet regular_fallback_chain = make_fallback_chain(font_weight::Regular, false);
    hilet bold_fallback_chain = make_fallback_chain(font_weight::Bold, false);
    hilet italic_fallback_chain = make_fallback_chain(font_weight::Regular, true);

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
                (fallback->italic == font->italic) and
                almost_equal(fallback->weight, font->weight)
            ) {
                fallback_chain.push_back(fallback);
            }
            // clang-format on
        }

        if (almost_equal(font->weight, font_weight::Bold)) {
            std::copy(begin(bold_fallback_chain), end(bold_fallback_chain), std::back_inserter(fallback_chain));
        } else if (font->italic == true) {
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

    auto i = _family_names.find(name);
    if (i == _family_names.end()) {
        hilet family_id = font_family_id(_font_variants.size());
        _font_variants.emplace_back();
        _family_names[name] = family_id;

        // If a new family is added, then the cache which includes fallbacks is no longer valid.
        _family_name_cache.clear();
        return family_id;
    } else {
        return i->second;
    }
}

[[nodiscard]] generator<std::string> font_book::generate_family_names(std::string_view name) const noexcept
{
    auto name_ = to_lower(name);

    // Lets try the first name first.
    co_yield name_;

    // Find the start of the chain.
    auto it = _family_name_fallback_chain.find(name_);
    if (it == _family_name_fallback_chain.end()) {
        it = _family_name_fallback_chain.find("fallback");
    }

    // Walk trough the chain.
    for (; it != _family_name_fallback_chain.end(); it = _family_name_fallback_chain.find(it->second)) {
        co_yield it->second;
    }
}

[[nodiscard]] font_family_id font_book::find_family(std::string_view family_name) const noexcept
{
    for (auto name : generate_family_names(family_name)) {
        if (hilet it = _family_name_cache.find(name); it != _family_name_cache.end()) {
            return it->second;
        }

        if (hilet it = _family_names.find(name); it != _family_names.end()) {
            _family_name_cache[name] = it->second;
            return it->second;
        }
    }

    hi_log_fatal("font_book::find_family() was unable to find a fallback font for '{}'.", family_name);
}

[[nodiscard]] font const& font_book::find_font(font_family_id family_id, font_variant variant) const noexcept
{
    hi_assert(family_id);
    hi_assert_bounds(*family_id, _font_variants);
    hilet& variants = _font_variants[*family_id];
    for (auto i = 0; i < 16; i++) {
        if (auto font = variants[variant.alternative(i)]) {
            return *font;
        }
    }
    // If a family exists, there must be at least one font variant available.
    hi_no_default();
}

[[nodiscard]] font const& font_book::find_font(font_family_id family_id, font_weight weight, bool italic) const noexcept
{
    return find_font(family_id, font_variant(weight, italic));
}

[[nodiscard]] font const& font_book::find_font(std::string_view family_name, font_weight weight, bool italic) const noexcept
{
    return find_font(find_family(family_name), weight, italic);
}

[[nodiscard]] glyph_ids font_book::find_glyph(hi::font const& font, grapheme g) const noexcept
{
    auto key = font_grapheme_id{font, g};

    auto i = _glyph_cache.find(key);
    if (i != _glyph_cache.end()) {
        return i->second;
    }

    // First try the selected font.
    if (hilet glyph_ids = font.find_glyph(g); not glyph_ids.empty()) {
        return _glyph_cache[key] = hi::glyph_ids{font, glyph_ids};
    }

    // Scan fonts which are fallback to this.
    for (hilet fallback : font.fallback_chain) {
        hi_axiom_not_null(fallback);
        if (hilet glyph_ids = fallback->find_glyph(g); not glyph_ids.empty()) {
            return _glyph_cache[key] = hi::glyph_ids{*fallback, glyph_ids};
        }
    }

    // If all everything has failed, use the tofu block of the original font.
    return _glyph_cache[key] = hi::glyph_ids{font, glyph_id{0}};
}

[[nodiscard]] font_book::estimate_run_result_type font_book::estimate_run(font const& font, gstring run) const noexcept
{
    auto r = estimate_run_result_type{};
    r.reserve(run.size());

    for (hilet grapheme: run) {
        hilet glyphs = find_glyph(font, grapheme);
        hilet &font = glyphs.font();

        r.fonts.push_back(&font);
        r.advances.push_back(font.get_advance(get<0>(glyphs)));
    }

    return r;
}

} // namespace hi::inline v1
