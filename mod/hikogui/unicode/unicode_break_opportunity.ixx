// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <format>
#include <ostream>
#include <vector>

export module hikogui_unicode_unicode_break_opportunity;
import hikogui_utility;


export namespace hi::inline v1 {

enum class unicode_break_opportunity : uint8_t {
    no,
    yes,
    mandatory,
    unassigned,
};

using unicode_break_vector = std::vector<unicode_break_opportunity>;
using unicode_break_iterator = unicode_break_vector::iterator;
using unicode_break_const_iterator = unicode_break_vector::const_iterator;

std::ostream &operator<<(std::ostream &lhs, unicode_break_opportunity const &rhs) {
    hilet *s = [&] () {
        switch (rhs) {
            using enum unicode_break_opportunity;
        case no: return "X";
        case yes: return ":";
        case mandatory: return "!";
        case unassigned: return "-";
        default: hi_no_default();
        }
    }();
    return lhs << s;
}

}

// XXX #617 MSVC bug does not handle partial specialization in modules.
export template<>
struct std::formatter<hi::unicode_break_opportunity, char> : std::formatter<char const *, char> {
    auto format(hi::unicode_break_opportunity const &t, auto &fc) const
    {
        hilet *s = [&]() {
            switch (t) {
            using enum hi::unicode_break_opportunity;
            case no: return "X";
            case yes: return ":";
            case mandatory: return "!";
            case unassigned: return "-";
            default: hi_no_default();
            }
        }();
        return std::formatter<char const *, char>::format(s, fc);
    }
};
