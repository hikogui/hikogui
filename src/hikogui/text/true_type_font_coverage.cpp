// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "true_type_font.hpp"
#include "../geometry/vector.hpp"
#include "../geometry/point.hpp"
#include "../placement.hpp"
#include "../strings.hpp"
#include "../endian.hpp"
#include "../log.hpp"
#include "../assert.hpp"
#include <cstddef>
#include <span>

namespace hi::inline v1 {

struct coverage_format1 {
    big_int16_buf_t coverage_format;
    big_int16_buf_t glyph_count;
};

struct coverage_format2 {
    big_int16_buf_t coverage_format;
    big_int16_buf_t range_count;
};

struct coverage_format2_range {
    big_int16_buf_t start_glyph_id;
    big_int16_buf_t end_glyph_id;
    big_int16_buf_t start_coverage_index;
};

[[nodiscard]] std::ptrdiff_t true_type_font::get_coverage_index(std::span<std::byte const> bytes, hi::glyph_id glyph_id) noexcept
{
    std::size_t offset = 0;

    hi_assert_or_return(glyph_id >= 0 && glyph_id < num_glyphs, -2);

    hi_assert_or_return(check_placement_ptr<coverage_format1>(bytes, offset), -2);
    hilet header1 = unsafe_make_placement_ptr<coverage_format1>(bytes, offset);
    if (header1->coverage_format == 1) {
        hi_assert_or_return(check_placement_array<big_uint16_buf_t>(bytes, offset, header1->glyph_count), -2);
        hilet table = unsafe_make_placement_array<big_uint16_buf_t>(bytes, offset, header1->glyph_count);

        hilet it = std::lower_bound(table.begin(), table.end(), glyph_id, [](hilet &item, hilet &value) {
            return item < *value;
        });

        if (it != table.end() and *it == *glyph_id) {
            return std::distance(table.begin(), it);
        } else {
            return -1;
        }

    } else if (header1->coverage_format == 2) {
        offset = 0;
        hi_assert_or_return(check_placement_ptr<coverage_format2>(bytes, offset), -2);
        hilet header2 = unsafe_make_placement_ptr<coverage_format2>(bytes, offset);

        hi_assert_or_return(check_placement_array<coverage_format2_range>(bytes, offset, header2->range_count), -2);
        hilet table = unsafe_make_placement_array<coverage_format2_range>(bytes, offset, header2->range_count);

        hilet it = std::lower_bound(table.begin(), table.end(), glyph_id, [](hilet &item, hilet &value) {
            return item.end_glyph_id < *value;
        });

        if (it != table.end() and it->start_glyph_id <= *glyph_id and *glyph_id <= it->end_glyph_id) {
            return it->start_coverage_index + *glyph_id - it->start_glyph_id;
        } else {
            return -1;
        }

    } else {
        return -2;
    }
}

} // namespace hi::inline v1
