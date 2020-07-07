// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/text/FontBook.hpp"
#include "ttauri/text/TrueTypeFont.hpp"
#include "ttauri/foundation/trace.hpp"

namespace tt {

FontBook::FontBook(std::vector<URL> const &font_directories)
{
    create_family_name_fallback_chain();

    for (ttlet &font_directory: font_directories) {
        ttlet font_directory_glob = font_directory / "**" / "*.ttf";
        for (ttlet &font_url: font_directory_glob.urlsByScanningWithGlobPattern()) {
            struct font_scan_tag {};
            auto t = trace<font_scan_tag>{};

            try {
                register_font(font_url, false);
            } catch (error &) {
                LOG_ERROR("Failed parsing font at {}", font_url);
            }
        }
    }

    post_process();
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

FontID FontBook::register_font(URL url, bool post_process)
{
    auto font = parseResource<Font>(url);
    auto &description = font->description;

    LOG_INFO("Parsed font {}: {}", url, to_string(description));

    ttlet font_id = FontID(ssize(font_entries));
    font_entries.emplace_back(url, description);

    ttlet font_family_id = register_family(description.family_name);
    font_variants[font_family_id][description.font_variant()] = font_id;

    if (post_process) {
        this->post_process();
    }

    return font_id;
}

void FontBook::calculate_fallback_fonts(FontEntry &entry, std::function<bool(FontDescription const&,FontDescription const&)> predicate) noexcept
{
    // First calculate total_ranges for the current fallback fonts.
    UnicodeRanges total_ranges = entry.description.unicode_ranges;
    for (ttlet fallback_id: entry.fallbacks) {
        total_ranges |= font_entries[fallback_id].description.unicode_ranges;
    }

    // Repeatably find the font that matches the predicate and improves the
    // total_ranges most by being included in the fallback list.
    while (true) {
        ssize_t max_font_id = -1;
        int max_popcount = total_ranges.popcount();

        // Find a font that matches the predicate and has the largest improvement.
        for (ssize_t fallback_id = 0; fallback_id != ssize(font_entries); ++fallback_id) {
            ttlet &fallback_entry = font_entries[fallback_id];

            if (!predicate(entry.description, fallback_entry.description)) {
                continue;
            }

            auto current_range = total_ranges | fallback_entry.description.unicode_ranges;
            auto current_popcount = current_range.popcount();

            if (current_popcount > max_popcount) {
                max_font_id = fallback_id;
                max_popcount = current_popcount;
            }
        }

        // Add the new best fallback font, or stop.
        if (max_font_id >= 0) {
            ttlet &fallback_entry = font_entries[max_font_id];
            //LOG_DEBUG("   {} - {}", fallback_entry.description.family_name, fallback_entry.description.sub_family_name);

            entry.fallbacks.push_back(FontID{max_font_id});
            total_ranges |= fallback_entry.description.unicode_ranges;
        } else {
            return;
        }
    }
    tt_unreachable();
}

void FontBook::post_process() noexcept
{
    // Reset caches.
    glyph_cache.clear();
    family_name_cache = family_names;

    // For each font, find fallback list.
    for (ssize_t i = 0; i != ssize(font_entries); ++i) {
        auto &entry = font_entries[i];
        entry.fallbacks.clear();

        //LOG_DEBUG("Looking for fallback fonts for: {}", to_string(entry.description));
        calculate_fallback_fonts(entry, [](ttlet &current, ttlet &fallback) {
            return
                starts_with(fallback.family_name, current.family_name) &&
                (current.italic == fallback.italic) &&
                almost_equal(current.weight, fallback.weight);
        });
        calculate_fallback_fonts(entry, [](ttlet &current, ttlet &fallback) {
            return
                (current.monospace == fallback.monospace) &&
                (current.serif == fallback.serif) &&
                (current.condensed == fallback.condensed) &&
                (current.italic == fallback.italic) &&
                almost_equal(current.weight, fallback.weight);
        });
        calculate_fallback_fonts(entry, [](ttlet &current, ttlet &fallback) {
            return true;
        });
    }
}

[[nodiscard]] FontFamilyID FontBook::register_family(std::string_view family_name) noexcept
{
    auto name = to_lower(family_name);

    auto i = family_names.find(name);
    if (i == family_names.end()) {
        ttlet family_id = FontFamilyID(ssize(font_variants));
        font_variants.emplace_back();
        family_names[name] = family_id;

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
        auto j = family_name_fallback_chain.find("fallback");
        tt_assert(j != family_name_fallback_chain.end());
        return j->second;
    }
}

[[nodiscard]] FontFamilyID FontBook::find_family(std::string_view family_name) const noexcept
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

[[nodiscard]] FontID FontBook::find_font(FontFamilyID family_id, FontVariant variant) const noexcept
{
    tt_assert(family_id);
    tt_assume(family_id >= 0 && family_id < ssize(font_variants));
    ttlet &variants = font_variants[family_id];
    for (auto i = 0; i < 16; i++) {
        if (auto font_id = variants[variant.alternative(i)]) {
            return font_id;
        }
    }
    // If a family exists, there must be at least one font variant available.
    tt_no_default;
}

[[nodiscard]] FontID FontBook::find_font(FontFamilyID family_id, FontWeight weight, bool italic) const noexcept
{
    return find_font(family_id, FontVariant(weight, italic));
}

[[nodiscard]] FontID FontBook::find_font(std::string_view family_name, FontWeight weight, bool italic) const noexcept
{
    return find_font(find_family(family_name), weight, italic);
}

[[nodiscard]] Font const &FontBook::get_font(FontID font_id) const noexcept
{
    tt_assume(font_id < ssize(font_entries));
    ttlet &entry = font_entries[font_id];

    if (!entry.font) {
        // This font was parsed once before, it must not give an error now.
        entry.font = parseResource<Font>(entry.url);
        tt_assert(entry.font);
    }

    return *(entry.font);
}

[[nodiscard]] FontGlyphIDs FontBook::find_glyph_actual(FontID font_id, Grapheme grapheme) const noexcept
{
    ttlet &font = get_font(font_id);

    auto glyph_ids = font.find_glyph(grapheme);
    glyph_ids.set_font_id(font_id);
    return glyph_ids;
}

[[nodiscard]] FontGlyphIDs FontBook::find_glyph(FontID font_id, Grapheme g) const noexcept
{
    auto i = glyph_cache.find({font_id, g});
    if (i != glyph_cache.end()) {
        return i->second;
    }

    // First try the selected font.
    auto glyph_ids = find_glyph_actual(font_id, g);
    if (glyph_ids) {
        glyph_cache[{font_id, g}] = glyph_ids;
        return glyph_ids;
    }

    // Scan fonts which are fallback to this.
    auto g_range = UnicodeRanges(g);
    for (ttlet fallback_id: font_entries[font_id].fallbacks) {
        auto &fallback_description = font_entries[fallback_id].description;
        if (fallback_description.unicode_ranges >= g_range) {
            if ((glyph_ids = find_glyph_actual(fallback_id, g))) {
                glyph_cache[{font_id, g}] = glyph_ids;
                return glyph_ids;
            }
        }
    }

    // If all everything has failed, use the tofu block of the original font.
    glyph_ids += GlyphID{0};
    glyph_ids.set_font_id(font_id);
    glyph_cache[{font_id, g}] = glyph_ids;
    return glyph_ids;
}



};
