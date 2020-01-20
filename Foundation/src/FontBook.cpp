// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Foundation/FontBook.hpp"
#include "TTauri/Foundation/trace.hpp"
#include "TTauri/Foundation/TrueTypeFont.hpp"
#include "TTauri/Foundation/FileView.hpp"

namespace TTauri {

FontBook::FontBook(std::vector<URL> const &font_directories)
{
    create_family_name_fallback_chain();

    for (let &font_directory: font_directories) {
        let font_directory_glob = font_directory / "**" / "*.ttf";
        for (let &font_url: font_directory_glob.urlsByScanningWithGlobPattern()) {
            auto t = trace<"font_scan"_tag>{};

            try {
                register_font(font_url);
            } catch (error &) {
                LOG_ERROR("Failed parsing font at {}", font_url);
            }
        }
    }
    // Pre-polutate the cache with the current families.
    family_name_cache = family_name_table;

    
    // Calculate fallback fonts.
    /*
    UnicodeRanges total_ranges;
    while (true) {
        int max_font_id = -1;
        int max_popcount = total_ranges.popcount();

        int current_font_id = 0;
        for (auto i = entries.begin(); i != entries.end(); ++i, ++current_font_id) {
            if (!(i->italic == false && i->monospace == false && i->weight == font_weight::Regular)) {
                continue;
            }

            auto current_range = total_ranges | i->unicode_ranges;
            auto current_popcount = current_range.popcount();

            if (current_popcount > max_popcount) {
                max_font_id = current_font_id;
                max_popcount = current_popcount;
            }
        }

        if (max_font_id >= 0) {
            let &best_entry = entries[max_font_id];
            LOG_INFO("Found fallback-font: id={}: {}", max_font_id, to_string(best_entry));

            last_resort_font_ids.push_back(max_font_id);
            total_ranges |= best_entry.unicode_ranges;
        } else {
            break;
        }
    }
    */
}

void FontBook::create_family_name_fallback_chain() noexcept
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

FontID FontBook::register_font(URL url)
{
    auto font = Font::load(url);
    auto &description = font->description;

    description.url = url;

    LOG_INFO("Parsed font {}: {}", url, to_string(description));

    let font_id = FontID(ssize(font_descriptions));
    font_descriptions.push_back(description);

    let font_family_id = register_family(description.family_name);
    font_variant_table[font_family_id][description.font_variant()] = font_id;
    return font_id;
}

[[nodiscard]] FontFamilyID FontBook::register_family(std::string_view family_name) noexcept
{
    auto name = to_lower(family_name);

    auto i = family_name_table.find(name);
    if (i == family_name_table.end()) {
        let family_id = FontFamilyID(ssize(font_variant_table));
        font_variant_table.emplace_back();
        family_name_table[name] = family_id;

        // If a new family is added, then the cache which includes fallbacks is no longer valid.
        family_name_cache.clear();
        return family_id;
    } else {
        return i->second;
    }
}

[[nodiscard]] std::string const &FontBook::find_fallback_family_name(std::string const &name) const noexcept
{
    auto i = family_name_fallback_chain.find(name);
    if (i != family_name_fallback_chain.end()) {
        return i->second;

    } else {
        auto i = family_name_fallback_chain.find("fallback");
        ttauri_assert(i != family_name_fallback_chain.end());
        return i->second;
    }
}

[[nodiscard]] FontFamilyID FontBook::find_family(std::string_view family_name) const noexcept
{
    let original_name = to_lower(family_name);

    let i = family_name_cache.find(original_name);
    if (i != family_name_cache.end()) {
        return i->second;
    }

    std::string const *name = &original_name;
    while (true) {
        name = &(find_fallback_family_name(*name));

        let i = family_name_table.find(*name);
        if (i != family_name_table.end()) {
            family_name_cache[original_name] = i->second;
            return i->second;
        }
    }
}

[[nodiscard]] FontID FontBook::find_font(FontFamilyID family_id, FontVariant variant) const noexcept
{
    ttauri_assert(family_id);
    ttauri_assume(family_id >= 0 && family_id < ssize(font_variant_table));
    let &font_variants = font_variant_table[family_id];
    for (auto i = 0; i < 16; i++) {
        if (auto font_id = font_variants[variant.alternative(i)]) {
            return font_id;
        }
    }
    // If a family exists, there must be at least one font variant available.
    no_default;
}

[[nodiscard]] FontID FontBook::find_font(FontFamilyID family_id, FontWeight weight, bool italic) const noexcept
{
    return find_font(family_id, FontVariant(weight, italic));
}

[[nodiscard]] FontID FontBook::find_font(std::string_view family_name, FontWeight weight, bool italic) const noexcept
{
    return find_font(find_family(family_name), weight, italic);
}

[[nodiscard]] FontGlyphIDs FontBook::get_glyph_exact(FontID font_id, grapheme grapheme) const noexcept
{
    auto font_description = font_descriptions[font_id];
    if (!font_description.font) {
        font_description.font = Font::load(font_description.url);
    }

    auto glyph_ids = font_description.font->get_glyphs(grapheme);
    glyph_ids.set_font_id(font_id);
    return glyph_ids;
}

[[nodiscard]] FontGlyphIDs FontBook::get_glyph(FontID font_id, grapheme grapheme) const noexcept
{
    // First try the selected font.
    auto glyph_ids = get_glyph_exact(font_id, grapheme);
    if (glyph_ids) {
        return glyph_ids;
    }

    // Now scan the fallback fonts.
    for (fb_font_id: fallback_font_table) {
        auto &font_description = font_descriptions[font_id];
        if (font_description.unicode_ranges.contains(grapheme)) {
            if ((glyph_ids = get_glyph_exeact(fb_font_id, grapheme))) {
                return glyph_ids;
            }
        }
    }

    // If all everything has failed, use the tofu block of the original font.
    glyph_ids += GlyphID{0};
    glyph_ids.set_font_id(font_id);
    return glyph_ids;
}


};
