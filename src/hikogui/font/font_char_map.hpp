// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file font/font_char_map.hpp Defined font_char_map type.
 * @ingroup font
 */

#pragma once

#include "glyph_id.hpp"
#include "../utility/module.hpp"
#include <cstdint>
#include <vector>
#include <tuple>
#include <algorithm>

namespace hi { inline namespace v1 {

/** Character map of a font.
 *
 * This type serves the check if a code-point is supported by a font
 * (even when the font is unloaded), and to retrieve the glyph mapped to
 * the code-point.
 *
 * @ingroup font
 */
class font_char_map {
public:
    constexpr font_char_map() noexcept = default;
    constexpr font_char_map(font_char_map const&) noexcept = default;
    constexpr font_char_map(font_char_map&&) noexcept = default;
    constexpr font_char_map& operator=(font_char_map const&) noexcept = default;
    constexpr font_char_map& operator=(font_char_map&&) noexcept = default;

    /** Reserve space for a set of ranges to be added.
     * @param n The number of ranges to be added.
     */
    void reserve(size_t n)
    {
        return _map.reserve(n);
    }

    /** Add a range of code points.
     *
     * @param start_code_point The starting code-point of the range.
     * @param start_glyph The starting glyph of the range.
     * @param count The number of code-points in this range.
     */
    [[nodiscard]] constexpr void add(char32_t start_code_point, uint32_t start_glyph, size_t count) noexcept
    {
#ifndef NDEBUG
        _prepared = false;
#endif

        while (count != 0) {
            hilet short_count = std::min(count, size_t{0x10000});
            hi_axiom(short_count != 0);

            _map.push_back(make_item(start_code_point, start_glyph, short_count));

            count -= short_count;
            start_code_point += narrow_cast<char32_t>(short_count);
            start_glyph += narrow_cast<uint32_t>(short_count);
        }
    }

    /** Prepare the map for searching.
     *
     * @pre All ranged of code-point must have been added with `add()`.
     * @post The character map can be searched using `find()`.
     */
    void prepare() noexcept
    {
        if (_map.empty()) {
            return;
        }

        std::sort(_map.begin(), _map.end());

        auto it = _map.begin();
        auto prev_it = it++;
        while (it != _map.end()) {
            hilet[p_code_point, p_glyph, p_count] = split_item(*prev_it);
            hilet[c_code_point, c_glyph, c_count] = split_item(*it);

            hi_axiom(p_code_point + p_count <= c_code_point);

            if (p_count + c_count <= 0x10000 and p_code_point + p_count == c_code_point and p_glyph + p_count == c_glyph) {
                *prev_it = make_item(p_code_point, p_glyph, p_count + c_count);
                it = _map.erase(it);

            } else {
                prev_it = it++;
            }
        }

        std::reverse(_map.begin(), _map.end());
        _map.shrink_to_fit();

#ifndef NDEBUG
        _prepared = true;
#endif
    }

    /** Find a glyph for a code_point.
     *
     * @param code_point The code-point to find in the character map.
     * @return The corrosponding glyph found representing the code-point, or an empty glyph if not found.
     */
    [[nodiscard]] constexpr glyph_id find(char32_t code_point) const noexcept
    {
#ifndef NDEBUG
        hi_assert(_prepared);
#endif

        hilet key = wide_cast<uint64_t>(code_point) << 32 | 0xffff'ffff;

        // A faster upper-bound search with less branches that are more predictable.
        auto base = _map.data();
        auto len = _map.size();
        while (len > 1) {
            hi_axiom_not_null(base);

            hilet half = len / 2;
            if (base[half - 1] > key) {
                base += half;
            }
            len -= half;
        }

        // Extract manually so that count does not need to be incremented.
        auto item = *base;
        auto item_glyph = truncate<uint16_t>(item);
        item >>= 16;
        auto item_count = truncate<uint16_t>(item);
        item >>= 16;
        auto item_code_point = truncate<char32_t>(item);

        hilet delta = code_point - item_code_point;

        if (delta <= item_count) {
            return glyph_id{item_glyph + delta};
        } else {
            // The key falls in a gap.
            return glyph_id{};
        }
    }

private:
    /** A std::make_heap list of code-point-range to glyph id.
     *
     * Each 64-bit integer is used as follows:
     *  - [63:43] 21-bit unicode start code-point.
     *  - [42:32] 11-bit count + 1 number of code-points encoded (zero count, means one code-point).
     *  - [31:0] 32-bit start glyph-id (0xffff is reserved).
     */
    std::vector<uint64_t> _map = {};

#ifndef NDEBUG
    bool _prepared = false;
#endif

    [[nodiscard]] constexpr static uint64_t make_item(char32_t code_point, uint16_t glyph, size_t count) noexcept
    {
        hi_axiom(count != 0 and count <= 0x10000);

        auto item = wide_cast<uint64_t>(code_point);
        item <<= 16;
        item |= count - 1;
        item <<= 16;
        item |= glyph;
        return item;
    }

    [[nodiscard]] constexpr static std::tuple<char32_t, uint16_t, size_t> split_item(uint64_t item) noexcept
    {
        hilet glyph = truncate<uint16_t>(item);
        item >>= 16;
        hilet count = wide_cast<size_t>(truncate<uint16_t>(item)) + 1;
        item >>= 16;
        hilet code_point = truncate<char32_t>(item);
        return {code_point, glyph, count};
    }
};

}} // namespace hi::v1
