// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file unicode/unicode_word_break.hpp
 */

#pragma once

#include "unicode_break_opportunity.hpp"
#include "ucd_general_categories.hpp"
#include "ucd_grapheme_cluster_breaks.hpp"
#include "ucd_word_break_properties.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <algorithm>
#include <vector>
#include <iterator>

hi_export_module(hikogui.unicode.unicode_word_break);


hi_export namespace hi::inline v1 {

namespace detail {

class unicode_word_break_info {
public:
    constexpr unicode_word_break_info() noexcept : _value(0) {}
    constexpr unicode_word_break_info(unicode_word_break_info const&) noexcept = default;
    constexpr unicode_word_break_info(unicode_word_break_info&&) noexcept = default;
    constexpr unicode_word_break_info& operator=(unicode_word_break_info const&) noexcept = default;
    constexpr unicode_word_break_info& operator=(unicode_word_break_info&&) noexcept = default;

    constexpr unicode_word_break_info(unicode_word_break_property const& word_break_property, bool pictographic) noexcept :
        _value(std::to_underlying(word_break_property) | (wide_cast<uint8_t>(pictographic) << 7))
    {
    }

    constexpr unicode_word_break_info& make_skip() noexcept
    {
        _value |= 0x40;
        return *this;
    }

    [[nodiscard]] constexpr bool is_skip() const noexcept
    {
        return to_bool(_value & 0x40);
    }

    [[nodiscard]] constexpr bool is_pictographic() const noexcept
    {
        return to_bool(_value & 0x80);
    }

    [[nodiscard]] constexpr friend bool
    operator==(unicode_word_break_info const& lhs, unicode_word_break_property const& rhs) noexcept
    {
        return (lhs._value & 0x3f) == std::to_underlying(rhs);
    }

    [[nodiscard]] constexpr friend bool
    operator==(unicode_word_break_info const&, unicode_word_break_info const&) noexcept = default;

    [[nodiscard]] constexpr friend bool is_AHLetter(unicode_word_break_info const& rhs) noexcept
    {
        return rhs == unicode_word_break_property::ALetter or rhs == unicode_word_break_property::Hebrew_Letter;
    }

    [[nodiscard]] constexpr friend bool is_MidNumLetQ(unicode_word_break_info const& rhs) noexcept
    {
        return rhs == unicode_word_break_property::MidNumLet or rhs == unicode_word_break_property::Single_Quote;
    }

private:
    uint8_t _value;
};

hi_inline void
unicode_word_break_WB1_WB3d(unicode_break_vector& r, std::vector<unicode_word_break_info>& infos) noexcept
{
    using enum unicode_break_opportunity;
    using enum unicode_word_break_property;

    hi_axiom(r.size() == infos.size() + 1);

    r.front() = yes; // WB1
    r.back() = yes; // WB2

    for (auto i = 1_uz; i < infos.size(); ++i) {
        hilet prev = infos[i - 1];
        hilet next = infos[i];

        r[i] = [&]() {
            if (prev == CR and next == LF) {
                return no; // WB3
            } else if (prev == Newline or prev == CR or prev == LF) {
                return yes; // WB3a
            } else if (next == Newline or next == CR or next == LF) {
                return yes; // WB3b
            } else if (prev == ZWJ and next.is_pictographic()) {
                return no; // WB3c
            } else if (prev == WSegSpace and next == WSegSpace) {
                return no; // WB3d
            } else {
                return unassigned;
            }
        }();
    }
}

hi_inline void unicode_word_break_WB4(unicode_break_vector& r, std::vector<unicode_word_break_info>& infos) noexcept
{
    using enum unicode_break_opportunity;
    using enum unicode_word_break_property;

    hi_axiom(r.size() == infos.size() + 1);

    for (auto i = 1_uz; i < infos.size(); ++i) {
        hilet prev = infos[i - 1];
        auto& next = infos[i];

        if ((prev != Newline and prev != CR and prev != LF) and (next == Extend or next == Format or next == ZWJ)) {
            if (r[i] == unassigned) {
                r[i] = no;
            }
            next.make_skip();
        }
    }
}

hi_inline void
unicode_word_break_WB5_WB999(unicode_break_vector& r, std::vector<unicode_word_break_info>& infos) noexcept
{
    using enum unicode_break_opportunity;
    using enum unicode_word_break_property;

    hi_axiom(r.size() == infos.size() + 1);

    for (auto i = 0_uz; i != infos.size(); ++i) {
        if (r[i] != unassigned) {
            continue;
        }

        hilet& next = infos[i];

        // WB4: (Extend | Format | ZWJ)* is assigned to no-break.
        hi_axiom(not next.is_skip());

        auto prev_i = narrow_cast<ptrdiff_t>(i) - 1;
        auto prev = unicode_word_break_info{};
        for (; prev_i >= 0 ; --prev_i) {
            if (not infos[prev_i].is_skip()) {
                prev = infos[prev_i];
                break;
            }
        }

        auto prev_prev_i = prev_i - 1;
        auto prev_prev = unicode_word_break_info{};
        for (; prev_prev_i >= 0; --prev_prev_i) {
            if (not infos[prev_prev_i].is_skip()) {
                prev_prev = infos[prev_prev_i];
                break;
            }
        }

        auto next_next_i = i + 1;
        auto next_next = unicode_word_break_info{};
        for (; next_next_i != infos.size(); ++next_next_i) {
            if (not infos[next_next_i].is_skip()) {
                next_next = infos[next_next_i];
                break;
            }
        }

        auto RI_i = prev_i - 1;
        auto RI_is_pair = true;
        if (prev == Regional_Indicator and next == Regional_Indicator) {
            // Track back before prev, and count consecutive RI.
            for (; RI_i >= 0; --RI_i) {
                if (infos[RI_i].is_skip()) {
                    continue;
                } else if (infos[RI_i] != Regional_Indicator) {
                    break;
                }
                RI_is_pair = not RI_is_pair;
            }
        }

        r[i] = [&] {
            if (is_AHLetter(prev) and is_AHLetter(next)) {
                return no; // WB5
            } else if (is_AHLetter(prev) and (next == MidLetter or is_MidNumLetQ(next)) and is_AHLetter(next_next)) {
                return no; // WB6
            } else if (is_AHLetter(prev_prev) and (prev == MidLetter or is_MidNumLetQ(prev)) and is_AHLetter(next)) {
                return no; // WB7
            } else if (prev == Hebrew_Letter and next == Single_Quote) {
                return no; // WB7a
            } else if (prev == Hebrew_Letter and next == Double_Quote and next_next == Hebrew_Letter) {
                return no; // WB7b
            } else if (prev_prev == Hebrew_Letter and prev == Double_Quote and next == Hebrew_Letter) {
                return no; // WB7c
            } else if (prev == Numeric and next == Numeric) {
                return no; // WB8
            } else if (is_AHLetter(prev) and next == Numeric) {
                return no; // WB9
            } else if (prev == Numeric and is_AHLetter(next)) {
                return no; // WB10
            } else if (prev_prev == Numeric and (prev == MidNum or is_MidNumLetQ(prev)) and next == Numeric) {
                return no; // WB11
            } else if (prev == Numeric and (next == MidNum or is_MidNumLetQ(next)) and next_next == Numeric) {
                return no; // WB12
            } else if (prev == Katakana and next == Katakana) {
                return no; // WB13
            } else if (
                (is_AHLetter(prev) or prev == Numeric or prev == Katakana or prev == ExtendNumLet) and next == ExtendNumLet) {
                return no; // WB13a
            } else if (prev == ExtendNumLet and (is_AHLetter(next) or next == Numeric or next == Katakana)) {
                return no; // WB13b
            } else if (prev == Regional_Indicator and next == Regional_Indicator and RI_is_pair) {
                return no; // WB15 WB16
            } else {
                return yes; // WB999
            }
        }();
    }
}

} // namespace detail

/** The unicode word break algorithm UAX#29
 *
 * @param first An iterator to the first character.
 * @param last An iterator to the last character.
 * @param code_point_func A function to code-point from a character.
 * @return A list of unicode_break_opportunity.
 */
template<typename It, typename ItEnd, typename CodePointFunc>
[[nodiscard]] hi_inline unicode_break_vector unicode_word_break(It first, ItEnd last, CodePointFunc const& code_point_func) noexcept
{
    auto size = narrow_cast<size_t>(std::distance(first, last));
    auto r = unicode_break_vector{size + 1, unicode_break_opportunity::unassigned};

    auto infos = std::vector<detail::unicode_word_break_info>{};
    infos.reserve(size);
    std::transform(first, last, std::back_inserter(infos), [&](hilet& item) {
        hilet code_point = code_point_func(item);
        hilet word_break_property = ucd_get_word_break_property(code_point);
        hilet grapheme_cluster_break = ucd_get_grapheme_cluster_break(code_point);
        return detail::unicode_word_break_info{
            word_break_property, grapheme_cluster_break == unicode_grapheme_cluster_break::Extended_Pictographic};
    });

    detail::unicode_word_break_WB1_WB3d(r, infos);
    detail::unicode_word_break_WB4(r, infos);
    detail::unicode_word_break_WB5_WB999(r, infos);
    return r;
}

/** Wrap lines in text that are too wide.
 * This algorithm may modify white-space in text and change them into line separators.
 * Lines are separated using the U+2028 code-point, and paragraphs are separated by
 * the U+2029 code-point.
 *
 * @param first The first iterator of a text to wrap
 * @param last The one beyond the last iterator of a text to wrap
 * @param max_width The maximum width of a line.
 * @param get_width A function returning the width of an item pointed by the iterator.
 *                  `float get_width(auto const &item)`
 * @param get_code_point A function returning the code-point of an item pointed by the iterator.
 *                       `char32_t get_code_point(auto const &item)`
 * @param set_code_point A function changing the code-point of an item pointed by the iterator.
 *                       `void set_code_point(auto &item, char32_t code_point)`
 */
void wrap_lines(auto first, auto last, float max_width, auto get_width, auto get_code_point, auto set_code_point) noexcept
{
    using enum unicode_general_category;

    auto it_at_last_space = last;
    float width_at_last_space = 0.0;
    float current_width = 0.0;

    for (auto it = first; it != last; ++it) {
        hilet code_point = get_code_point(*it);
        hilet general_category = ucd_get_general_category(code_point);

        if (general_category == Zp || general_category == Zl) {
            // Reset the line on existing line and paragraph separator.
            it_at_last_space = last;
            width_at_last_space = 0.0f;
            current_width = 0.0;
            continue;

        } else if (general_category == Zs) {
            // Remember the length of the line at the end of the word.
            it_at_last_space = it;
            width_at_last_space = current_width;
        }

        current_width += get_width(*it);
        if (current_width >= max_width && it_at_last_space != last) {
            // The line is too long, replace the last space with a line separator.
            set_code_point(*it, U'\u2028');
            it_at_last_space = last;
            width_at_last_space = 0.0f;
            current_width = 0.0;
            continue;
        }
    }
}

} // namespace hi::inline v1
