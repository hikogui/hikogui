// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include <format>
#include <ostream>
#include <vector>

namespace hi::inline v1 {

enum class unicode_break_opportunity : uint8_t {
    no,
    yes,
    mandatory,
    unassigned,
};

using unicode_break_vector = std::vector<unicode_break_opportunity>;
using unicode_break_iterator = unicode_break_vector::iterator;
using unicode_break_const_iterator = unicode_break_vector::const_iterator;

inline std::ostream &operator<<(std::ostream &lhs, unicode_break_opportunity const &rhs) {
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

template<typename CharT>
struct std::formatter<hi::unicode_break_opportunity, CharT> : std::formatter<char const *, CharT> {
    auto format(hi::unicode_break_opportunity const &t, auto &fc)
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
        return std::formatter<char const *, CharT>::format(s, fc);
    }
};
