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
#include "../unicode/unicode_mask.hpp"
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

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _map.empty();
    }

    /** Reserve space for a set of ranges to be added.
     * @param n The number of ranges to be added.
     */
    constexpr void reserve(size_t n)
    {
        return _map.reserve(n);
    }

    /** Add a range of code points.
     *
     * @param start_code_point The starting code-point of the range.
     * @param end_code_point The ending code-point of the range (inclusive).
     * @param start_glyph The starting glyph of the range.
     */
    [[nodiscard]] constexpr void add(char32_t start_code_point, char32_t end_code_point, uint16_t start_glyph) noexcept
    {
#ifndef NDEBUG
        _prepared = false;
#endif
        hi_axiom(start_code_point <= end_code_point);
        auto count = wide_cast<size_t>(end_code_point - start_code_point + 1);
        hi_axiom(start_glyph + count < 0xffff, "Only glyph_ids 0 through 0xfffe are valid");

        while (count != 0) {
            hilet short_count = std::min(count, entry_type::max_count);
            hi_axiom(short_count != 0);

            _map.emplace_back(start_code_point, char_cast<char32_t>(start_code_point + short_count - 1), start_glyph);

            count -= short_count;
            start_code_point += narrow_cast<char32_t>(short_count);
            start_glyph += narrow_cast<uint16_t>(short_count);
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
            return a.end_code_point < b.end_code_point;
        });

        auto it = _map.begin();
        auto prev_it = it++;
        while (it != _map.end()) {
            hi_axiom(prev_it->end_code_point < it->start_code_point());

            if (mergable(*prev_it, *it)) {
                hilet merged_count = std::min(prev_it->count() + it->count(), entry_type::max_count);
                hilet move_count = merged_count - prev_it->count();
                hi_axiom(move_count <= entry_type::max_count);

                prev_it->end_code_point += narrow_cast<char32_t>(move_count);
                prev_it->set_count(prev_it->count() + move_count);

                if (move_count == it->count()) {
                    it = _map.erase(it);
                    // Don't change prev or it; since we just deleted instead of advanced.
                    continue;

                } else {
                    hi_axiom(move_count < it->count());
                    it->start_glyph += narrow_cast<uint16_t>(move_count);
                    it->set_count(it->count() - move_count);
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

        if (hilet item_ptr = fast_lower_bound(std::span{_map}, char_cast<uint32_t>(code_point))) {
            return item_ptr->get(code_point);
        }
        return {};
    }

    [[nodiscard]] unicode_mask make_mask() const noexcept
    {
        auto r = unicode_mask();

        for (hilet& entry : _map) {
            r.add(entry.start_code_point(), entry.end_code_point + 1);
        }

        r.optimize();
        r.shrink_to_fit();
        return r;
    }

private:
    struct entry_type {
        constexpr static size_t max_count = 0x1'0000;

        char32_t end_code_point;
        uint16_t start_glyph;
        uint16_t _count;

        constexpr entry_type(char32_t start_code_point, char32_t end_code_point, uint16_t start_glyph) noexcept :
            end_code_point(end_code_point),
            start_glyph(start_glyph),
            _count(narrow_cast<uint16_t>(end_code_point - start_code_point))
        {
            hi_axiom(start_code_point <= end_code_point);
        }

        [[nodiscard]] constexpr size_t count() const noexcept
        {
            return wide_cast<size_t>(_count) + 1;
        }

        [[nodiscard]] constexpr void set_count(size_t new_count) noexcept
        {
            hi_axiom(new_count > 0);
            hi_axiom(new_count <= max_count);
            _count = narrow_cast<uint16_t>(new_count - 1);
        }

        [[nodiscard]] constexpr char32_t start_code_point() const noexcept
        {
            return end_code_point - _count;
        }

        [[nodiscard]] constexpr uint16_t end_glyph() const noexcept
        {
            return narrow_cast<uint16_t>(start_glyph + _count);
        }

        [[nodiscard]] constexpr friend bool mergable(entry_type const& lhs, entry_type const& rhs) noexcept
        {
            return lhs.end_code_point + 1 == rhs.start_code_point() and lhs.end_glyph() + 1 == rhs.start_glyph;
        }

        [[nodiscard]] constexpr glyph_id get(char32_t code_point) const noexcept
        {
            auto diff = wide_cast<ptrdiff_t>(code_point) - wide_cast<ptrdiff_t>(end_code_point);
            if (diff > 0) {
                return {};
            }
            diff += _count;
            if (diff < 0) {
                return {};
            }
            diff += start_glyph;
            return glyph_id{truncate<uint16_t>(diff)};
        }
    };

    std::vector<entry_type> _map = {};

#ifndef NDEBUG
    bool _prepared = false;
#endif
};

}} // namespace hi::v1
