// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "true_type_font.hpp"
#include "otype_utilities.hpp"
#include "otype_head.hpp"
#include "otype_hhea.hpp"
#include "otype_maxp.hpp"
#include "otype_name.hpp"
#include "otype_os2.hpp"
#include "otype_sfnt.hpp"
#include "otype_cmap.hpp"
#include "../geometry/module.hpp"
#include "../utility/module.hpp"
#include "../placement.hpp"
#include "../strings.hpp"
#include "../log.hpp"
#include <cstddef>
#include <span>


namespace hi::inline v1 {



struct KERNTable_ver0 {
    big_uint16_buf_t version;
    big_uint16_buf_t nTables;
};

struct KERNTable_ver1 {
    big_uint32_buf_t version;
    big_uint32_buf_t nTables;
};

struct KERNSubtable_ver0 {
    big_uint16_buf_t version;
    big_uint16_buf_t length;
    big_uint16_buf_t coverage;
};

struct KERNSubtable_ver1 {
    big_uint32_buf_t length;
    big_uint16_buf_t coverage;
    big_uint16_buf_t tupleIndex;
};

struct KERNFormat0 {
    big_uint16_buf_t nPairs;
    big_uint16_buf_t searchRange;
    big_uint16_buf_t entrySelector;
    big_uint16_buf_t rangeShift;
};

struct KERNFormat0_entry {
    big_uint16_buf_t left;
    big_uint16_buf_t right;
    otype_fword_buf_t value;
};

struct HMTXEntry {
    otype_fuword_buf_t advanceWidth;
    otype_fword_buf_t leftSideBearing;
};

struct GLYFEntry {
    big_int16_buf_t numberOfContours;
    otype_fword_buf_t xMin;
    otype_fword_buf_t yMin;
    otype_fword_buf_t xMax;
    otype_fword_buf_t yMax;
};

[[nodiscard]] glyph_id true_type_font::find_glyph(char32_t c) const
{
    return _char_map.find(c);
}

bool true_type_font::get_glyf_bytes(glyph_id glyph_id, std::span<std::byte const>& glyph_bytes) const
{
    load_view();
    hi_assert_or_return(*glyph_id >= 0 && *glyph_id < num_glyphs, false);

    std::size_t startOffset = 0;
    std::size_t endOffset = 0;
    if (_loca_is_offset32) {
        hilet entries = make_placement_array<big_uint32_buf_t>(_loca_table_bytes);
        hi_assert_or_return(static_cast<int>(glyph_id) + 1 < entries.size(), false);

        startOffset = *entries[*glyph_id];
        endOffset = *entries[*glyph_id + 1];

    } else {
        hilet entries = make_placement_array<big_uint16_buf_t>(_loca_table_bytes);
        hi_assert_or_return(static_cast<int>(glyph_id) + 1 < entries.size(), false);

        startOffset = *entries[*glyph_id] * 2;
        endOffset = *entries[*glyph_id + 1] * 2;
    }

    hi_assert_or_return(startOffset <= endOffset, false);
    hilet size = endOffset - startOffset;

    hi_assert_or_return(endOffset <= static_cast<std::size_t>(_glyf_table_bytes.size()), false);
    glyph_bytes = _glyf_table_bytes.subspan(startOffset, size);
    return true;
}

static void get_kern0_kerning(
    std::span<std::byte const> const& bytes,
    uint16_t coverage,
    float _em_scale,
    glyph_id glyph1_id,
    glyph_id glyph2_id,
    vector2& r)
{
    std::size_t offset = 0;

    hilet formatheader = make_placement_ptr<KERNFormat0>(bytes, offset);
    hilet nPairs = *formatheader->nPairs;

    hilet entries = make_placement_array<KERNFormat0_entry>(bytes, offset, nPairs);

    hilet i = std::lower_bound(entries.begin(), entries.end(), std::pair{glyph1_id, glyph2_id}, [](hilet& a, hilet& b) {
        if (*a.left == b.first) {
            return *a.right < *b.second;
        } else {
            return *a.left < *b.first;
        }
    });
    hi_assert_or_return(i != entries.end(), );

    if (glyph1_id == *i->left && glyph2_id == *i->right) {
        // Writing direction is assumed horizontal.
        switch (coverage & 0xf) {
        case 0x1:
            r.x() = r.x() + i->value * _em_scale;
            break;
        case 0x3:
            r.x() = std::min(r.x(), i->value * _em_scale);
            break;
        case 0x5:
            r.y() = r.y() + i->value * _em_scale;
            break;
        case 0x7:
            r.y() = std::min(r.y(), i->value * _em_scale);
            break;
        // Override
        case 0x9:
            r.x() = i->value * _em_scale;
            break;
        case 0xb:
            r.x() = i->value * _em_scale;
            break;
        case 0xd:
            r.y() = i->value * _em_scale;
            break;
        case 0xf:
            r.y() = i->value * _em_scale;
            break;
        default:;
        }
    }
}

static void get_kern3_kerning(
    std::span<std::byte const> const& bytes,
    uint16_t coverage,
    float _em_scale,
    glyph_id glyph1_id,
    glyph_id glyph2_id,
    vector2& r)
{
}

[[nodiscard]] static vector2
get_kern_kerning(std::span<std::byte const> const& bytes, float _em_scale, glyph_id glyph1_id, glyph_id glyph2_id)
{
    auto r = vector2{0.0f, 0.0f};
    std::size_t offset = 0;

    hilet header_ver0 = make_placement_ptr<KERNTable_ver0>(bytes, offset);
    uint32_t version = *header_ver0->version;

    uint32_t nTables = 0;
    if (version == 0x0000) {
        nTables = *header_ver0->nTables;

    } else {
        // Restart with version 1 table.
        offset = 0;
        hilet header_ver1 = make_placement_ptr<KERNTable_ver1>(bytes, offset);
        hi_assert_or_return(*header_ver1->version == 0x00010000, r);
        nTables = *header_ver1->nTables;
    }

    for (uint32_t subtableIndex = 0; subtableIndex != nTables; ++subtableIndex) {
        hilet subtable_offset = offset;

        uint16_t coverage = 0;
        uint32_t length = 0;
        if (version == 0x0000) {
            hilet subheader = make_placement_ptr<KERNSubtable_ver0>(bytes, offset);
            coverage = *subheader->coverage;
            length = *subheader->length;

        } else {
            hilet subheader = make_placement_ptr<KERNSubtable_ver1>(bytes, offset);
            coverage = *subheader->coverage;
            length = *subheader->length;
        }

        switch (coverage >> 8) {
        case 0: // Pairs
            get_kern0_kerning(bytes.subspan(offset), coverage, _em_scale, glyph1_id, glyph2_id, r);
            break;
        case 3: // Compact 2D kerning values.
            get_kern3_kerning(bytes.subspan(offset), coverage, _em_scale, glyph1_id, glyph2_id, r);
            break;
        }

        offset = subtable_offset + length;
    }

    return r;
}

[[nodiscard]] vector2 true_type_font::get_kerning(hi::glyph_id current_glyph, hi::glyph_id next_glyph) const
{
    load_view();

    if (not _kern_table_bytes.empty()) {
        return get_kern_kerning(_kern_table_bytes, _em_scale, current_glyph, next_glyph);
    } else {
        return vector2{0.0f, 0.0f};
    }
}

bool true_type_font::update_glyph_metrics(
    hi::glyph_id glyph_id,
    hi::glyph_metrics& glyph_metrics,
    hi::glyph_id kern_glyph1_id,
    hi::glyph_id kern_glyph2_id) const
{
    load_view();

    hi_assert_or_return(*glyph_id >= 0 && *glyph_id < num_glyphs, false);

    ssize_t offset = 0;

    hilet longHorizontalMetricTable = make_placement_array<HMTXEntry>(_hmtx_table_bytes, offset, numberOfHMetrics);

    hilet numberOfLeftSideBearings = num_glyphs - numberOfHMetrics;
    hilet leftSideBearings = make_placement_array<otype_fword_buf_t>(_hmtx_table_bytes, offset, numberOfLeftSideBearings);

    float advanceWidth = 0.0f;
    float leftSideBearing;
    if (*glyph_id < numberOfHMetrics) {
        advanceWidth = longHorizontalMetricTable[*glyph_id].advanceWidth * _em_scale;
        leftSideBearing = longHorizontalMetricTable[*glyph_id].leftSideBearing * _em_scale;
    } else {
        advanceWidth = longHorizontalMetricTable[numberOfHMetrics - 1].advanceWidth * _em_scale;
        leftSideBearing = leftSideBearings[*glyph_id - numberOfHMetrics] * _em_scale;
    }

    glyph_metrics.advance = vector2{advanceWidth, 0.0f};
    glyph_metrics.left_side_bearing = leftSideBearing;
    glyph_metrics.right_side_bearing = advanceWidth - (leftSideBearing + glyph_metrics.bounding_rectangle.width());

    if (kern_glyph1_id && kern_glyph2_id) {
        glyph_metrics.advance += get_kerning(kern_glyph1_id, kern_glyph2_id);
    }

    return true;
}

constexpr uint8_t FLAG_ON_CURVE = 0x01;
constexpr uint8_t FLAG_X_SHORT = 0x02;
constexpr uint8_t FLAG_Y_SHORT = 0x04;
constexpr uint8_t FLAG_REPEAT = 0x08;
constexpr uint8_t FLAG_X_SAME = 0x10;
constexpr uint8_t FLAG_Y_SAME = 0x20;
bool true_type_font::load_simple_glyph(std::span<std::byte const> glyph_bytes, graphic_path& glyph) const
{
    load_view();

    std::size_t offset = 0;

    hilet entry = make_placement_ptr<GLYFEntry>(glyph_bytes, offset);

    hilet numberOfContours = static_cast<std::size_t>(*entry->numberOfContours);

    // Check includes instructionLength.
    hilet endPoints = make_placement_array<big_uint16_buf_t>(glyph_bytes, offset, numberOfContours);

    int max_end_point = -1;
    for (hilet endPoint : endPoints) {
        // End points must be incrementing and contours must have at least one point.
        hi_assert_or_return(wide_cast<int>(*endPoint) >= max_end_point, false);
        max_end_point = wide_cast<int>(*endPoint);

        glyph.contourEndPoints.push_back(*endPoint);
    }

    hilet numberOfPoints = *endPoints[numberOfContours - 1] + 1;

    // Skip over the instructions.
    hilet instructionLength = **make_placement_ptr<big_uint16_buf_t>(glyph_bytes, offset);
    offset += instructionLength * ssizeof(uint8_t);

    // Extract all the flags.
    std::vector<uint8_t> flags;
    flags.reserve(numberOfPoints);
    while (flags.size() < numberOfPoints) {
        hilet flag = *make_placement_ptr<uint8_t>(glyph_bytes, offset);

        flags.push_back(flag);
        if (flag & FLAG_REPEAT) {
            hilet repeat = *make_placement_ptr<uint8_t>(glyph_bytes, offset);

            for (std::size_t i = 0; i < repeat; i++) {
                flags.push_back(flag);
            }
        }
    }
    hi_assert_or_return(flags.size() == numberOfPoints, false);

    hilet point_table_size = std::accumulate(flags.begin(), flags.end(), static_cast<std::size_t>(0), [](auto size, auto flag) {
        return size + ((flag & FLAG_X_SHORT) > 0 ? 1 : ((flag & FLAG_X_SAME) > 0 ? 0 : 2)) +
            ((flag & FLAG_Y_SHORT) > 0 ? 1 : ((flag & FLAG_Y_SAME) > 0 ? 0 : 2));
    });
    hi_assert_or_return(offset + point_table_size <= static_cast<std::size_t>(glyph_bytes.size()), false);

    // Get xCoordinates
    std::vector<int16_t> xCoordinates;
    xCoordinates.reserve(numberOfPoints);
    for (hilet flag : flags) {
        if ((flag & FLAG_X_SHORT) > 0) {
            if ((flag & FLAG_X_SAME) > 0) {
                xCoordinates.push_back(static_cast<int16_t>(*make_placement_ptr<uint8_t>(glyph_bytes, offset)));
            } else {
                // Negative short.
                xCoordinates.push_back(-static_cast<int16_t>(*make_placement_ptr<uint8_t>(glyph_bytes, offset)));
            }
        } else {
            if ((flag & FLAG_X_SAME) > 0) {
                xCoordinates.push_back(0);
            } else {
                // Long
                xCoordinates.push_back(**make_placement_ptr<big_int16_buf_t>(glyph_bytes, offset));
            }
        }
    }

    // Get yCoordinates
    std::vector<int16_t> yCoordinates;
    yCoordinates.reserve(numberOfPoints);
    for (hilet flag : flags) {
        if ((flag & FLAG_Y_SHORT) > 0) {
            if ((flag & FLAG_Y_SAME) > 0) {
                yCoordinates.push_back(static_cast<int16_t>(*make_placement_ptr<uint8_t>(glyph_bytes, offset)));
            } else {
                // Negative short.
                yCoordinates.push_back(-static_cast<int16_t>(*make_placement_ptr<uint8_t>(glyph_bytes, offset)));
            }
        } else {
            if ((flag & FLAG_Y_SAME) > 0) {
                yCoordinates.push_back(0);
            } else {
                // Long
                yCoordinates.push_back(**make_placement_ptr<big_int16_buf_t>(glyph_bytes, offset));
            }
        }
    }

    // Create absolute points
    int16_t x = 0;
    int16_t y = 0;
    std::size_t pointNr = 0;
    std::vector<bezier_point> points;
    points.reserve(numberOfPoints);
    for (hilet flag : flags) {
        x += xCoordinates[pointNr];
        y += yCoordinates[pointNr];

        hilet type = (flag & FLAG_ON_CURVE) > 0 ? bezier_point::Type::Anchor : bezier_point::Type::QuadraticControl;

        glyph.points.emplace_back(x * _em_scale, y * _em_scale, type);
        pointNr++;
    }

    return true;
}

constexpr uint16_t FLAG_ARG_1_AND_2_ARE_WORDS = 0x0001;
constexpr uint16_t FLAG_ARGS_ARE_XY_VALUES = 0x0002;
[[maybe_unused]] constexpr uint16_t FLAG_ROUND_XY_TO_GRID = 0x0004;
constexpr uint16_t FLAG_WE_HAVE_A_SCALE = 0x0008;
constexpr uint16_t FLAG_MORE_COMPONENTS = 0x0020;
constexpr uint16_t FLAG_WE_HAVE_AN_X_AND_Y_SCALE = 0x0040;
constexpr uint16_t FLAG_WE_HAVE_A_TWO_BY_TWO = 0x0080;
[[maybe_unused]] constexpr uint16_t FLAG_WE_HAVE_INSTRUCTIONS = 0x0100;
constexpr uint16_t FLAG_USE_MY_METRICS = 0x0200;
[[maybe_unused]] constexpr uint16_t FLAG_OVERLAP_COMPOUND = 0x0400;
constexpr uint16_t FLAG_SCALED_COMPONENT_OFFSET = 0x0800;
[[maybe_unused]] constexpr uint16_t FLAG_UNSCALED_COMPONENT_OFFSET = 0x1000;
bool true_type_font::load_compound_glyph(std::span<std::byte const> glyph_bytes, graphic_path& glyph, glyph_id& metrics_glyph_id)
    const
{
    load_view();

    std::size_t offset = ssizeof(GLYFEntry);

    uint16_t flags;
    do {
        flags = **make_placement_ptr<big_uint16_buf_t>(glyph_bytes, offset);

        hilet subGlyphIndex = **make_placement_ptr<big_uint16_buf_t>(glyph_bytes, offset);

        graphic_path subGlyph;
        hi_assert_or_return(load_glyph(glyph_id{subGlyphIndex}, subGlyph), false);

        auto subGlyphOffset = vector2{};
        if (flags & FLAG_ARGS_ARE_XY_VALUES) {
            if (flags & FLAG_ARG_1_AND_2_ARE_WORDS) {
                hilet tmp = make_placement_array<otype_fword_buf_t>(glyph_bytes, offset, 2);
                subGlyphOffset = vector2{tmp[0] * _em_scale, tmp[1] * _em_scale};
            } else {
                hilet tmp = make_placement_array<otype_fbyte_buf_t>(glyph_bytes, offset, 2);
                subGlyphOffset = vector2{tmp[0] * _em_scale, tmp[1] * _em_scale};
            }
        } else {
            std::size_t pointNr1;
            std::size_t pointNr2;
            if (flags & FLAG_ARG_1_AND_2_ARE_WORDS) {
                hilet tmp = make_placement_array<big_uint16_buf_t>(glyph_bytes, offset, 2);
                pointNr1 = *tmp[0];
                pointNr2 = *tmp[1];
            } else {
                hilet tmp = make_placement_array<uint8_t>(glyph_bytes, offset, 2);
                pointNr1 = tmp[0];
                pointNr2 = tmp[1];
            }
            // XXX Implement
            hi_log_warning("Reading glyph from font with !FLAG_ARGS_ARE_XY_VALUES");
            return false;
        }

        // Start with an identity matrix.
        auto subGlyphScale = scale2{};
        if (flags & FLAG_WE_HAVE_A_SCALE) {
            subGlyphScale = scale2(make_placement_ptr<otype_fixed1_14_buf_t>(glyph_bytes, offset)->value());

        } else if (flags & FLAG_WE_HAVE_AN_X_AND_Y_SCALE) {
            hilet tmp = make_placement_array<otype_fixed1_14_buf_t>(glyph_bytes, offset, 2);
            subGlyphScale = scale2(tmp[0].value(), tmp[1].value());

        } else if (flags & FLAG_WE_HAVE_A_TWO_BY_TWO) {
            hilet tmp = make_placement_array<otype_fixed1_14_buf_t>(glyph_bytes, offset, 4);
            hi_not_implemented();
            // subGlyphScale = mat::S(
            //    tmp[0].value(),
            //    tmp[1].value(),
            //    tmp[2].value(),
            //    tmp[3].value()
            //)
        }

        if (flags & FLAG_SCALED_COMPONENT_OFFSET) {
            subGlyphOffset = subGlyphScale * subGlyphOffset;
        }

        if (flags & FLAG_USE_MY_METRICS) {
            metrics_glyph_id = subGlyphIndex;
        }

        glyph += translate2(subGlyphOffset) * subGlyphScale * subGlyph;

    } while (flags & FLAG_MORE_COMPONENTS);
    // Ignore trailing instructions.

    return true;
}

std::optional<glyph_id> true_type_font::load_glyph(glyph_id glyph_id, graphic_path& glyph) const
{
    load_view();

    hi_assert_or_return(*glyph_id >= 0 && *glyph_id < num_glyphs, {});

    std::span<std::byte const> glyph_bytes;
    hi_assert_or_return(get_glyf_bytes(glyph_id, glyph_bytes), {});

    auto metrics_glyph_id = glyph_id;

    if (glyph_bytes.size() > 0) {
        hilet entry = make_placement_ptr<GLYFEntry>(glyph_bytes);
        hilet numberOfContours = *entry->numberOfContours;

        hi_assert_or_return((entry->xMin * 1.0f) <= (entry->xMax * 1.0f), {});
        hi_assert_or_return((entry->yMin * 1.0f) <= (entry->yMax * 1.0f), {});

        if (numberOfContours > 0) {
            hi_assert_or_return(load_simple_glyph(glyph_bytes, glyph), {});
        } else if (numberOfContours < 0) {
            hi_assert_or_return(load_compound_glyph(glyph_bytes, glyph, metrics_glyph_id), {});
        } else {
            // Empty glyph, such as white-space ' '.
        }

    } else {
        // Empty glyph, such as white-space ' '.
    }

    return metrics_glyph_id;
}

bool true_type_font::load_compound_glyph_metrics(std::span<std::byte const> bytes, glyph_id& metrics_glyph_id) const
{
    load_view();

    std::size_t offset = ssizeof(GLYFEntry);

    uint16_t flags;
    do {
        flags = **make_placement_ptr<big_uint16_buf_t>(bytes, offset);

        hilet subGlyphIndex = **make_placement_ptr<big_uint16_buf_t>(bytes, offset);

        if (flags & FLAG_ARGS_ARE_XY_VALUES) {
            if (flags & FLAG_ARG_1_AND_2_ARE_WORDS) {
                offset += ssizeof(otype_fword_buf_t) * 2;
            } else {
                offset += ssizeof(otype_fbyte_buf_t) * 2;
            }
        } else {
            if (flags & FLAG_ARG_1_AND_2_ARE_WORDS) {
                offset += ssizeof(big_uint16_buf_t) * 2;
            } else {
                offset += ssizeof(uint8_t) * 2;
            }
        }

        if (flags & FLAG_WE_HAVE_A_SCALE) {
            offset += ssizeof(otype_fixed1_14_buf_t);
        } else if (flags & FLAG_WE_HAVE_AN_X_AND_Y_SCALE) {
            offset += ssizeof(otype_fixed1_14_buf_t) * 2;
        } else if (flags & FLAG_WE_HAVE_A_TWO_BY_TWO) {
            offset += ssizeof(otype_fixed1_14_buf_t) * 4;
        }

        if (flags & FLAG_USE_MY_METRICS) {
            metrics_glyph_id = subGlyphIndex;
            return true;
        }
    } while (flags & FLAG_MORE_COMPONENTS);
    // Ignore trailing instructions.

    return true;
}

bool true_type_font::load_glyph_metrics(hi::glyph_id glyph_id, hi::glyph_metrics& glyph_metrics, hi::glyph_id lookahead_glyph_id)
    const
{
    load_view();

    hi_assert_or_return(*glyph_id >= 0 && *glyph_id < num_glyphs, false);

    std::span<std::byte const> glyph_bytes;
    hi_assert_or_return(get_glyf_bytes(glyph_id, glyph_bytes), false);

    auto metricsGlyphIndex = glyph_id;

    if (glyph_bytes.size() > 0) {
        hilet entry = make_placement_ptr<GLYFEntry>(glyph_bytes);
        hilet numberOfContours = *entry->numberOfContours;

        hilet xyMin = point2{entry->xMin * _em_scale, entry->yMin * _em_scale};
        hilet xyMax = point2{entry->xMax * _em_scale, entry->yMax * _em_scale};
        glyph_metrics.bounding_rectangle = aarectangle{xyMin, xyMax};

        if (numberOfContours > 0) {
            // A simple glyph does not include metrics information in the data.
        } else if (numberOfContours < 0) {
            hi_assert_or_return(load_compound_glyph_metrics(glyph_bytes, metricsGlyphIndex), false);
        } else {
            // Empty glyph, such as white-space ' '.
        }

    } else {
        // Empty glyph, such as white-space ' '.
    }

    return update_glyph_metrics(metricsGlyphIndex, glyph_metrics, glyph_id, lookahead_glyph_id);
}

void true_type_font::parse_font_directory(std::span<std::byte const> bytes)
{
    if (auto head_bytes = otype_sfnt_search<"head">(bytes); not head_bytes.empty()) {
        auto head = otype_head_parse(head_bytes);
        _loca_is_offset32 = head.loca_is_offset32;
        _em_scale = head.em_scale;
    }

    if (auto name_bytes = otype_sfnt_search<"name">(bytes); not name_bytes.empty()) {
        auto names = otype_name_get_family(name_bytes);
        family_name = std::move(names.family_name);
        sub_family_name = std::move(names.sub_family_name);
    }

    if (auto maxp_bytes = otype_sfnt_search<"maxp">(bytes); not maxp_bytes.empty()) {
        auto maxp = otype_maxp_parse(maxp_bytes);
        num_glyphs = maxp.num_glyphs;
    }

    if (auto hhea_bytes = otype_sfnt_search<"hhea">(bytes); not hhea_bytes.empty()) {
        auto hhea = otype_hhea_parse(hhea_bytes, _em_scale);
        metrics.ascender = hhea.ascender;
        metrics.descender = -hhea.descender;
        metrics.line_gap = hhea.line_gap;
        numberOfHMetrics = hhea.number_of_h_metrics;
    }

    if (auto cmap_bytes = otype_sfnt_search<"cmap">(bytes); not cmap_bytes.empty()) {
        _char_map = otype_cmap_parse(cmap_bytes);
    } else {
        throw parse_error("Could not find 'cmap'");
    }

    if (auto os2_bytes = otype_sfnt_search<"OS/2">(bytes); not os2_bytes.empty()) {
        auto os2 = otype_parse_os2(os2_bytes, _em_scale);
        weight = os2.weight;
        condensed = os2.condensed;
        serif = os2.serif;
        monospace = os2.monospace;
        italic = os2.italic;
        OS2_x_height = os2.x_height;
        OS2_cap_height = os2.cap_height;
    }

    cache_tables(bytes);

    unicode_mask = _char_map.make_mask();

    // Parsing the weight, italic and other features from the sub-family-name
    // is much more reliable than the explicit data in the OS/2 table.
    // Only use the OS/2 data as a last resort.
    // clang-format off
    auto name_lower = to_lower(family_name + " " + sub_family_name);
    if (name_lower.find("italic") != std::string::npos or
        name_lower.find("oblique") != std::string::npos) {
        italic = true;
    }

    if (name_lower.find("condensed") != std::string::npos) {
        condensed = true;
    }

    if (name_lower.find("mono") != std::string::npos or
        name_lower.find("console") != std::string::npos or
        name_lower.find("code") != std::string::npos ) {
        monospace = true;
    }

    if (name_lower.find("sans") != std::string::npos) {
        serif = false;
    } else if (name_lower.find("serif") != std::string::npos) {
        serif = true;
    }

    if (name_lower.find("regular") != std::string::npos or
        name_lower.find("medium") != std::string::npos) {
        weight = font_weight::Regular;
    } else if (
        name_lower.find("extra light") != std::string::npos or
        name_lower.find("extra-light") != std::string::npos or
        name_lower.find("extralight") != std::string::npos) {
        weight = font_weight::ExtraLight;
    } else if (
        name_lower.find("extra black") != std::string::npos or
        name_lower.find("extra-black") != std::string::npos or
        name_lower.find("extrablack") != std::string::npos) {
        weight = font_weight::ExtraBlack;
    } else if (
        name_lower.find("extra bold") != std::string::npos or
        name_lower.find("extra-bold") != std::string::npos or
        name_lower.find("extrabold") != std::string::npos) {
        weight = font_weight::ExtraBold;
    } else if (name_lower.find("thin") != std::string::npos) {
        weight = font_weight::Thin;
    } else if (name_lower.find("light") != std::string::npos) {
        weight = font_weight::Light;
    } else if (name_lower.find("bold") != std::string::npos) {
        weight = font_weight::Bold;
    } else if (name_lower.find("black") != std::string::npos) {
        weight = font_weight::Black;
    }
    // clang-format on

    // Figure out the features.
    features.clear();
    if (not _kern_table_bytes.empty()) {
        features += "kern,";
    }
    if (not _GSUB_table_bytes.empty()) {
        features += "GSUB,";
    }

    if (OS2_x_height > 0.0f) {
        metrics.x_height = OS2_x_height;
    } else {
        hilet glyph_id = find_glyph('x');
        if (glyph_id) {
            hi::glyph_metrics glyph_metrics;
            load_glyph_metrics(glyph_id, glyph_metrics);
            metrics.x_height = glyph_metrics.bounding_rectangle.height();
        }
    }

    if (OS2_cap_height > 0.0f) {
        metrics.cap_height = OS2_cap_height;
    } else {
        hilet glyph_id = find_glyph('H');
        if (glyph_id) {
            hi::glyph_metrics glyph_metrics;
            load_glyph_metrics(glyph_id, glyph_metrics);
            metrics.cap_height = glyph_metrics.bounding_rectangle.height();
        }
    }

    hilet glyph_id = find_glyph('8');
    if (glyph_id) {
        hi::glyph_metrics glyph_metrics;
        load_glyph_metrics(glyph_id, glyph_metrics);
        metrics.digit_advance = glyph_metrics.advance.x();
    }
}

} // namespace hi::inline v1
