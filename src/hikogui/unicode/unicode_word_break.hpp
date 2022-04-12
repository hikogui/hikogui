// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file unicode/unicode_word_break.hpp
 */

#pragma once

#include "unicode_grapheme_cluster_break.hpp"
#include "unicode_break_opportunity.hpp"
#include "../cast.hpp"
#include <algorithm>
#include <vector>

namespace tt::inline v1{

    enum class unicode_word_break_property : uint8_t {
    Other,
    CR,
    LF,
    Newline,
    Extend,
    ZWJ,
    Regional_Indicator,
    Format,
    Katakana,
    Hebrew_Letter,
    ALetter,
    Single_Quote,
    Double_Quote,
    MidNumLet,
    MidLetter,
    MidNum,
    Numeric,
    ExtendNumLet,
    WSegSpace
};

namespace detail {

class unicode_word_break_info {
public:
    constexpr unicode_word_break_info() noexcept : _value(0)
    {}
    constexpr unicode_word_break_info(unicode_word_break_info const &) noexcept = default;
    constexpr unicode_word_break_info(unicode_word_break_info &&) noexcept = default;
    constexpr unicode_word_break_info &operator=(unicode_word_break_info const &) noexcept = default;
    constexpr unicode_word_break_info &operator=(unicode_word_break_info &&) noexcept = default;

    constexpr unicode_word_break_info(unicode_word_break_property const &word_break_property, bool pictographic) noexcept : _value(to_underlying(word_break_property) | (static_cast<uint8_t>(pictographic) << 7))
    {}

    constexpr unicode_word_break_info &make_skip() noexcept
    {
        _value |= 0x40;
        return *this;
    }

    [[nodiscard]] constexpr bool is_skip() const noexcept
    {
        return static_cast<bool>(_value & 0x40);
    }

    [[nodiscard]] constexpr bool is_pictographic() const noexcept
    {
        return static_cast<bool>(_value & 0x80);
    }

    [[nodiscard]] constexpr friend bool operator==(unicode_word_break_info const &lhs, unicode_word_break_property const &rhs) noexcept
    {
        return (lhs._value & 0x3f) == to_underlying(rhs);
    }

    [[nodiscard]] constexpr friend bool operator==(unicode_word_break_info const &, unicode_word_break_info const &) noexcept = default;

    [[nodiscard]] constexpr friend bool is_AHLetter(unicode_word_break_info const &rhs) noexcept
    {
        return rhs == unicode_word_break_property::ALetter or rhs == unicode_word_break_property::Hebrew_Letter;
    }

    [[nodiscard]] constexpr friend bool is_MidNumLetQ(unicode_word_break_info const &rhs) noexcept
    {
        return rhs == unicode_word_break_property::MidNumLet or rhs == unicode_word_break_property::Single_Quote;
    }

private:
    uint8_t _value;
};

[[nodiscard]] inline void unicode_word_break_WB1_WB3d(
    unicode_break_vector &r,
    std::vector<unicode_word_break_info> &infos) noexcept
{
    using enum unicode_break_opportunity;
    using enum unicode_word_break_property;

    tt_axiom(r.size() == infos.size() + 1);

    r.front() = yes; // WB1
    r.back() = yes; // WB2

    for (auto i = 1_uz; i < infos.size(); ++i) {
        ttlet prev = infos[i - 1];
        ttlet next = infos[i];

        r[i] = [&] () {
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

[[nodiscard]] inline void unicode_word_break_WB4(
    unicode_break_vector &r,
    std::vector<unicode_word_break_info> &infos) noexcept
{
    using enum unicode_break_opportunity;
    using enum unicode_word_break_property;

    tt_axiom(r.size() == infos.size() + 1);

    for (auto i = 1_uz; i < infos.size(); ++i) {
        ttlet prev = infos[i - 1];
        auto &next = infos[i];

        if ((prev != Newline and prev != CR and prev != LF) and (next == Extend or next == Format or next == ZWJ)) {
            if (r[i] == unassigned) {
                r[i] = no;
            }
            next.make_skip();
        }
    }
}

[[nodiscard]] inline void unicode_word_break_WB5_WB999(
    unicode_break_vector &r,
    std::vector<unicode_word_break_info> &infos) noexcept
{
    using enum unicode_break_opportunity;
    using enum unicode_word_break_property;

    tt_axiom(r.size() == infos.size() + 1);

    auto RI_count = 0_uz;
    ttlet size = narrow<std::ptrdiff_t>(infos.size());
    for (auto i = 0_z; i < size; ++i) {
        ttlet &next = infos[i];
        if (next == Regional_Indicator) {
            ++RI_count;
        } else {
            RI_count = 0;
        }

        if (r[i] != unassigned) {
            continue;
        }

        tt_axiom(not next.is_skip());

        std::ptrdiff_t k;

        ttlet prev = [&] {
            for (k = i - 1; k >= 0; --k) {
                if (not infos[k].is_skip()) {
                    return infos[k];
                }
            }
            return unicode_word_break_info{};
        }();

        ttlet prev_prev = [&] {
            for (--k; k >= 0; --k) {
                if (not infos[k].is_skip()) {
                    return infos[k];
                }
            }
            return unicode_word_break_info{};
        }();

        ttlet next_next = [&] {
            for (k = i + 1; k < size; ++k) {
                if (not infos[k].is_skip()) {
                    return infos[k];
                }
            }
            return unicode_word_break_info{};
        }();

        r[i] = [&] () {
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
            } else if ((is_AHLetter(prev) or prev == Numeric or prev == Katakana or prev == ExtendNumLet) and next == ExtendNumLet) {
                return no; // WB13a
            } else if (prev == ExtendNumLet and (is_AHLetter(next) or next == Numeric or next == Katakana)) {
                return no; // WB13b
            } else if (prev == Regional_Indicator and next == Regional_Indicator and (RI_count % 2) == 1) {
                return no; // WB15 WB16
            } else {
                return yes; // WB999
            }
        }();
    }
}

}

/** The unicode word break algorithm UAX#29
*
* @param first An iterator to the first character.
* @param last An iterator to the last character.
* @param description_func A function to get a reference to unicode_description from a character.
* @return A list of unicode_break_opportunity.
 */
template<typename It, typename ItEnd, typename DescriptionFunc>
[[nodiscard]] inline unicode_break_vector unicode_word_break(It first, ItEnd last, DescriptionFunc const &description_func)
{
    auto size = narrow<size_t>(std::distance(first, last));
    auto r = unicode_break_vector{size + 1, unicode_break_opportunity::unassigned};

    auto infos = std::vector<detail::unicode_word_break_info>{};
    infos.reserve(size);
    std::transform(first, last, std::back_inserter(infos), [&] (ttlet &item) {
        ttlet &description = description_func(item);
        return detail::unicode_word_break_info{description.word_break_property(), description.grapheme_cluster_break() == unicode_grapheme_cluster_break::Extended_Pictographic};
        });

    detail::unicode_word_break_WB1_WB3d(r, infos);
    detail::unicode_word_break_WB4(r, infos);
    detail::unicode_word_break_WB5_WB999(r, infos);
    return r;
}

}
