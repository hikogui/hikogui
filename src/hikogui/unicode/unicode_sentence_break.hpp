// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file unicode/unicode_sentence_break.hpp
 */

#pragma once

#include "ucd_sentence_break_properties.hpp"
#include "unicode_break_opportunity.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <tuple>
#include <vector>
#include <iterator>
#include <algorithm>

hi_export_module(hikogui.unicode.unicode_sentence_break);

hi_export namespace hi::inline v1 {

namespace detail {

class unicode_sentence_break_info {
public:
    constexpr unicode_sentence_break_info() noexcept : _value(0)
    {}
    constexpr unicode_sentence_break_info(unicode_sentence_break_info const &) noexcept = default;
    constexpr unicode_sentence_break_info(unicode_sentence_break_info &&) noexcept = default;
    constexpr unicode_sentence_break_info &operator=(unicode_sentence_break_info const &) noexcept = default;
    constexpr unicode_sentence_break_info &operator=(unicode_sentence_break_info &&) noexcept = default;

    constexpr unicode_sentence_break_info(unicode_sentence_break_property const &sentence_break_property) noexcept : _value(std::to_underlying(sentence_break_property))
    {}

    constexpr unicode_sentence_break_info &make_skip() noexcept
    {
        _value |= 0x40;
        return *this;
    }

    [[nodiscard]] constexpr bool is_skip() const noexcept
    {
        return to_bool(_value & 0x40);
    }

    [[nodiscard]] constexpr friend bool operator==(unicode_sentence_break_info const &lhs, unicode_sentence_break_property const &rhs) noexcept
    {
        return (lhs._value & 0x3f) == std::to_underlying(rhs);
    }

    [[nodiscard]] constexpr friend bool operator==(unicode_sentence_break_info const &, unicode_sentence_break_info const &) noexcept = default;

    [[nodiscard]] constexpr friend bool is_ParaSep(unicode_sentence_break_info const &rhs) noexcept
    {
        return rhs == unicode_sentence_break_property::Sep or rhs == unicode_sentence_break_property::CR or rhs == unicode_sentence_break_property::LF;
    }

    [[nodiscard]] constexpr friend bool is_SATerm(unicode_sentence_break_info const &rhs) noexcept
    {
        return rhs == unicode_sentence_break_property::STerm or rhs == unicode_sentence_break_property::ATerm;
    }

private:
    uint8_t _value;
};

hi_inline void unicode_sentence_break_SB1_SB4(
    unicode_break_vector &r,
    std::vector<unicode_sentence_break_info> &infos) noexcept
{
    using enum unicode_break_opportunity;
    using enum unicode_sentence_break_property;

    hi_axiom(r.size() == infos.size() + 1);

    r.front() = yes; // SB1
    r.back() = yes; // SB2

    for (auto i = 1_uz; i < infos.size(); ++i) {
        hilet prev = infos[i - 1];
        hilet next = infos[i];

        r[i] = [&] () {
            if (prev == CR and next == LF) {
                return no; // SB3
            } else if (is_ParaSep(prev)) {
                return yes; //SB4
            } else {
                return unassigned;
            }
        }();
    }
}

hi_inline void unicode_sentence_break_SB5(
    unicode_break_vector &r,
    std::vector<unicode_sentence_break_info> &infos) noexcept
{
    using enum unicode_break_opportunity;
    using enum unicode_sentence_break_property;

    hi_axiom(r.size() == infos.size() + 1);

    for (auto i = 1_uz; i < infos.size(); ++i) {
        hilet prev = infos[i - 1];
        auto &next = infos[i];

        if ((not is_ParaSep(prev) and prev != CR and prev != LF) and (next == Extend or next == Format)) {
            if (r[i] == unassigned) {
                r[i] = no;
            }
            next.make_skip();
        }
    }
}

hi_inline void unicode_sentence_break_SB6_SB998(
    unicode_break_vector &r,
    std::vector<unicode_sentence_break_info> &infos) noexcept
{
    using enum unicode_break_opportunity;
    using enum unicode_sentence_break_property;

    hi_axiom(r.size() == infos.size() + 1);

    for (auto i = 0_z; i < std::ssize(infos); ++i) {
        hilet &next = infos[i];
        if (r[i] != unassigned) {
            continue;
        }

        hi_axiom(not next.is_skip());

        std::ptrdiff_t k;

        hilet prev = [&] {
            for (k = i - 1; k >= 0; --k) {
                if (not infos[k].is_skip()) {
                    return infos[k];
                }
            }
            return unicode_sentence_break_info{};
        }();

        hilet prev_prev = [&] {
            for (--k; k >= 0; --k) {
                if (not infos[k].is_skip()) {
                    return infos[k];
                }
            }
            return unicode_sentence_break_info{};
        }();

        // close_sp
        // 0 - no suffix
        // 1 - ends in ParSep
        // 2 - includes SP
        // 4 - includes Close
        hilet [prefix, close_sp_par_found] = [&]() {
            using enum unicode_break_opportunity;

            auto found = 0;
            auto state = ' ';
            for (auto j = i - 1; j >= 0; --j) {
                if (not infos[j].is_skip()) {
                    switch (state) {
                    case ' ':
                        if (is_ParaSep(infos[j])) {
                            found |= 1;
                            state = 'p';
                        } else if (infos[j] == Sp) {
                            found |= 2;
                            state = 's';
                        } else if (infos[j] == Close) {
                            found |= 4;
                            state = 'c';
                        } else {
                            return std::make_pair(infos[j], found);
                        }
                        break;
                    case 'p': // We can only be in the state 'p' once.
                    case 's':
                        if (infos[j] == Sp) {
                            found |= 2;
                            state = 's';
                        } else if (infos[j] == Close) {
                            found |= 4;
                            state = 'c';
                        } else {
                            return std::make_pair(infos[j], found);
                        }
                        break;
                    case 'c':
                        if (infos[j] == Close) {
                            found |= 4;
                            state = 'c';
                        } else {
                            return std::make_pair(infos[j], found);
                        }
                        break;
                    }
                }
            }
            return std::make_pair(unicode_sentence_break_info{}, 0);
        }();
        hilet optional_close = (close_sp_par_found & 3) == 0;
        hilet optional_close_sp = (close_sp_par_found & 1) == 0;
        hilet optional_close_sp_par = true;

        hilet end_in_lower = [&]{
            for (auto j = i; j < std::ssize(infos); ++j) {
                if (not infos[j].is_skip()) {
                    if (infos[j] == Lower) {
                        return true;
                    } else if (infos[j] == OLetter or infos[j] == Upper or is_ParaSep(infos[j]) or is_SATerm(infos[j])) {
                        return false;
                    }
                }
            }
            return false;
        }();

        r[i] = [&] () {
            if (prev == ATerm and next == Numeric) {
                return no; // SB6
            } else if ((prev_prev == Upper or prev_prev == Lower) and prev == ATerm and next == Upper) {
                return no; // SB7
            } else if (prefix == ATerm and optional_close_sp and end_in_lower) {
                return no; // SB8
            } else if (is_SATerm(prefix) and optional_close_sp and (next == SContinue or is_SATerm(next))) {
                return no; // SB8a
            } else if (is_SATerm(prefix) and optional_close and (next == Close or next == Sp or is_ParaSep(next))) {
                return no; // SB9
            } else if (is_SATerm(prefix) and optional_close_sp and (next == Sp or is_ParaSep(next))) {
                return no; // SB10
            } else if (is_SATerm(prefix) and optional_close_sp_par) {
                return yes; // SB11
            } else {
                return no; // SB998
            }
        }();
    }
}

}

/** The unicode word break algorithm UAX#29
*
* @param first An iterator to the first character.
* @param last An iterator to the last character.
* @param code_point_func A function to get a code-point from an dereferenced iterator.
* @return A list of unicode_break_opportunity before each character.
 */
template<typename It, typename ItEnd, typename CodePointFunc>
[[nodiscard]] hi_inline unicode_break_vector
unicode_sentence_break(It first, ItEnd last, CodePointFunc const& code_point_func) noexcept
{
    auto size = narrow_cast<size_t>(std::distance(first, last));
    auto r = unicode_break_vector{size + 1, unicode_break_opportunity::unassigned};

    auto infos = std::vector<detail::unicode_sentence_break_info>{};
    infos.reserve(size);
    std::transform(first, last, std::back_inserter(infos), [&] (hilet &item) {
        hilet code_point = code_point_func(item);
        return detail::unicode_sentence_break_info{ucd_get_sentence_break_property(code_point)};
        });

    detail::unicode_sentence_break_SB1_SB4(r, infos);
    detail::unicode_sentence_break_SB5(r, infos);
    detail::unicode_sentence_break_SB6_SB998(r, infos);
    return r;
}


}
