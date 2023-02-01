// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file font/font_char_map.hpp Defined font_char_map type.
 * @ingroup font
 */

#pragma once

#include "glyph_id.hpp"
#include "../algorithm.hpp"
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
    [[nodiscard]] constexpr void add(char32_t start_code_point, uint16_t start_glyph, size_t count) noexcept
    {
#ifndef NDEBUG
        _prepared = false;
#endif

        while (count != 0) {
            hilet short_count = truncate<uint16_t>(std::min(count, size_t{0xffff}));
            hi_axiom(short_count != 0);

            _map.emplace_back(start_code_point, start_glyph, short_count);

            count -= short_count;
            start_code_point += short_count;
            start_glyph += short_count;
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

        // Sort the entries in reverse order so that the lower_bound search becomes upper_bound.
        std::sort(_map.begin(), _map.end(), [](hilet& a, hilet& b) {
            return a.code_point < b.code_point;
        });

        auto it = _map.begin();
        auto prev_it = it++;
        while (it != _map.end()) {
            hi_axiom(prev_it->code_point + prev_it->count <= it->code_point);

            if (prev_it->code_point + prev_it->count == it->code_point and prev_it->glyph + prev_it->count == it->glyph) {
                hilet merged_count = wide_cast<uint32_t>(prev_it->count) + wide_cast<uint32_t>(it->count);
                if (merged_count <= 0xffff) {
                    prev_it->count = truncate<uint16_t>(merged_count);
                    it = _map.erase(it);
                    // Don't change prev or it; since we just deleted instead of advanced.
                    continue;

                } else {
                    hilet moved_count = truncate<uint16_t>(0xffff - prev_it->count);
                    prev_it->count = 0xffff;
                    it->code_point += moved_count;
                    it->glyph += moved_count;
                    it->count -= moved_count;
                }
            }

            prev_it = it++;
        }

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
    [[nodiscard]] inline glyph_id find(char32_t code_point) const noexcept
    {
#ifndef NDEBUG
        hi_assert(_prepared);
#endif

        if (hilet item_ptr = fast_binary_search_le(std::span{_map}, char_cast<uint32_t>(code_point))) {
            hilet delta = code_point - item_ptr->code_point;
            if (delta >= item_ptr->count) {
                return glyph_id{};
            }

            return glyph_id{item_ptr->glyph + delta};

        } else {
            return glyph_id{};
        }
    }

private:
    struct item_type {
        char32_t code_point;
        uint16_t glyph;
        uint16_t count;
    };

    std::vector<item_type> _map = {};

#ifndef NDEBUG
    bool _prepared = false;
#endif
};

}} // namespace hi::v1
