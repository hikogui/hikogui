// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "font_book.hpp"
#include "true_type_font.hpp"
#include "../trace.hpp"
#include "../ranges.hpp"
#include "../log.hpp"
#include <ranges>
#include <vector>

namespace tt::inline v1 {

font_book::font_book(std::vector<URL> const &font_directories)
{
    create_family_name_fallback_chain();

    for (ttlet &font_directory : font_directories) {
        ttlet font_directory_glob = font_directory / "**" / "*.ttf";
        for (ttlet &font_url : font_directory_glob.urlsByScanningWithGlobPattern()) {
            auto t = trace<"font_scan">{};

            try {
                register_font(font_url, false);

            } catch (std::exception const &e) {
                tt_log_error("Failed parsing font at {}: \"{}\"", font_url, e.what());
            }
        }
    }
}

void font_book::create_family_name_fallback_chain() noexcept
{
    family_name_fallback_chain["fallback"] = "sans-serif";

    // Serif web-fonts
    family_name_fallback_chain["serif"] = "times new roman";
    family_name_fallback_chain["times new roman"] = "times";
    family_name_fallback_chain["times"] = "noto serif";
    family_name_fallback_chain["noto serif"] = "noto";

    family_name_fallback_chain["georgia"] = "serif";

    family_name_fallback_chain["palatino"] = "palatino linotype";
    family_name_fallback_chain["palatino linotype"] = "book antiqua";
    family_name_fallback_chain["book antiqua"] = "serif";

    // Sans-serif web-fonts
    family_name_fallback_chain["sans-serif"] = "arial";
    family_name_fallback_chain["arial"] = "helvetica";
    family_name_fallback_chain["helvetica"] = "noto sans";

    family_name_fallback_chain["gadget"] = "sans-sarif";

    family_name_fallback_chain["comic sans"] = "comic sans ms";
    family_name_fallback_chain["comic sans ms"] = "cursive";
    family_name_fallback_chain["cursive"] = "sans-serif";

    family_name_fallback_chain["impact"] = "charcoal";
    family_name_fallback_chain["charcoal"] = "sans-serif";

    family_name_fallback_chain["lucida"] = "lucida sans";
    family_name_fallback_chain["lucida sans"] = "lucida sans unicode";
    family_name_fallback_chain["lucida sans unicode"] = "lucida grande";
    family_name_fallback_chain["lucida grande"] = "sans-serif";

    family_name_fallback_chain["verdana"] = "geneva";
    family_name_fallback_chain["tahoma"] = "geneva";
    family_name_fallback_chain["geneva"] = "sans-serif";

    family_name_fallback_chain["trebuchet"] = "trebuchet ms";
    family_name_fallback_chain["trebuchet ms"] = "helvetica";
    family_name_fallback_chain["helvetic"] = "sans-serif";

    // Monospace web-fonts.
    family_name_fallback_chain["monospace"] = "courier";
    family_name_fallback_chain["courier"] = "courier new";

    family_name_fallback_chain["consolas"] = "lucida console";
    family_name_fallback_chain["lucida console"] = "monaco";
    family_name_fallback_chain["monaco"] = "andale mono";
    family_name_fallback_chain["andale mono"] = "monospace";
}

font &font_book::register_font(URL url, bool post_process)
{
    auto font = std::make_unique<true_type_font>(url);
    auto font_ptr = font.get();

    tt_log_info("Parsed font {}: {}", url, to_string(*font));

    ttlet font_family_id = register_family(font->family_name);
    font_variants[*font_family_id][font->font_variant()] = font_ptr;

    _fonts.emplace_back(std::move(font));
    _font_ptrs.push_back(font_ptr);

    if (post_process) {
        this->post_process();
    }

    return *font_ptr;
}

[[nodiscard]] std::vector<tt::font *> font_book::make_fallback_chain(font_weight weight, bool italic) noexcept
{
    auto r = _font_ptrs;

    std::stable_partition(begin(r), end(r), [weight, italic](ttlet &item) {
        return (item->italic == italic) and almost_equal(item->weight, weight);
    });

    auto unicode_mask = tt::unicode_mask{};
    for (auto &font : r) {
        if (not unicode_mask.contains(font->unicode_mask)) {
            // This font adds unicode code points.
            unicode_mask |= font->unicode_mask;

        } else {
            font = nullptr;
        }
    }

    std::erase(r, nullptr);
    return r;
}

void font_book::post_process() noexcept
{
    // Reset caches and fallback chains.
    glyph_cache.clear();
    family_name_cache = family_names;

    // Sort the list of fonts based on the amount of unicode code points it supports.
    std::sort(begin(_font_ptrs), end(_font_ptrs), [](ttlet &lhs, ttlet &rhs) {
        return lhs->unicode_mask.size() > rhs->unicode_mask.size();
    });

    ttlet regular_fallback_chain = make_fallback_chain(font_weight::Regular, false);
    ttlet bold_fallback_chain = make_fallback_chain(font_weight::Bold, false);
    ttlet italic_fallback_chain = make_fallback_chain(font_weight::Regular, true);

    tt_log_info(
        "Post processing fonts number={}, regular-fallback={}, bold-fallback={}, italic-fallback={}",
        size(_fonts),
        size(regular_fallback_chain),
        size(bold_fallback_chain),
        size(italic_fallback_chain));

    // For each font, find fallback list.
    for (ttlet &font : _font_ptrs) {
        auto fallback_chain = std::vector<tt::font *>{};

        // Put the fonts from the same family, italic and weight first.
        for (ttlet &fallback : _font_ptrs) {
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

    auto i = family_names.find(name);
    if (i == family_names.end()) {
        ttlet family_id = font_family_id(font_variants.size());
        font_variants.emplace_back();
        family_names[name] = family_id;

        // If a new family is added, then the cache which includes fallbacks is no longer valid.
        family_name_cache.clear();
        return family_id;
    } else {
        return i->second;
    }
}

[[nodiscard]] std::string const &font_book::find_fallback_family_name(std::string const &name) const noexcept
{
    auto i = family_name_fallback_chain.find(name);
    if (i != family_name_fallback_chain.end()) {
        return i->second;

    } else {
        auto j = family_name_fallback_chain.find("fallback");
        tt_assert(j != family_name_fallback_chain.end());
        return j->second;
    }
}

[[nodiscard]] font_family_id font_book::find_family(std::string_view family_name) const noexcept
{
    ttlet original_name = to_lower(family_name);

    ttlet i = family_name_cache.find(original_name);
    if (i != family_name_cache.end()) {
        return i->second;
    }

    std::string const *name = &original_name;
    while (true) {
        name = &(find_fallback_family_name(*name));

        ttlet j = family_names.find(*name);
        if (j != family_names.end()) {
            family_name_cache[original_name] = j->second;
            return j->second;
        }
    }
}

[[nodiscard]] font const &font_book::find_font(font_family_id family_id, font_variant variant) const noexcept
{
    tt_assert(family_id);
    tt_axiom(family_id >= 0 && family_id < ssize(font_variants));
    ttlet &variants = font_variants[*family_id];
    for (auto i = 0; i < 16; i++) {
        if (auto font = variants[variant.alternative(i)]) {
            return *font;
        }
    }
    // If a family exists, there must be at least one font variant available.
    tt_no_default();
}

[[nodiscard]] font const &font_book::find_font(font_family_id family_id, font_weight weight, bool italic) const noexcept
{
    return find_font(family_id, font_variant(weight, italic));
}

[[nodiscard]] font const &font_book::find_font(std::string_view family_name, font_weight weight, bool italic) const noexcept
{
    return find_font(find_family(family_name), weight, italic);
}

[[nodiscard]] glyph_ids font_book::find_glyph(tt::font const &font, grapheme g) const noexcept
{
    auto key = font_grapheme_id{font, g};

    auto i = glyph_cache.find(key);
    if (i != glyph_cache.end()) {
        return i->second;
    }

    // First try the selected font.
    auto glyph_ids = font.find_glyph(g);
    if (glyph_ids) {
        glyph_cache[key] = glyph_ids;
        return glyph_ids;
    }

    // Scan fonts which are fallback to this.
    for (ttlet fallback : font.fallback_chain) {
        if (glyph_ids = fallback->find_glyph(g)) {
            glyph_cache[key] = glyph_ids;
            return glyph_ids;
        }
    }

    // If all everything has failed, use the tofu block of the original font.
    glyph_ids += glyph_id{0};
    glyph_ids.set_font(font);
    glyph_cache[key] = glyph_ids;
    return glyph_ids;
}

} // namespace tt::inline v1
