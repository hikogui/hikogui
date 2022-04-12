// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../assert.hpp"
#include <format>
#include <ostream>
#include <vector>

namespace tt::inline v1 {

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
    ttlet *s = [&] () {
        switch (rhs) {
            using enum unicode_break_opportunity;
        case no: return "X";
        case yes: return ":";
        case mandatory: return "!";
        case unassigned: return "-";
        default: tt_no_default();
        }
    }();
    return lhs << s;
}

}

template<typename CharT>
struct std::formatter<tt::unicode_break_opportunity, CharT> : std::formatter<char const *, CharT> {
    auto format(tt::unicode_break_opportunity const &t, auto &fc)
    {
        ttlet *s = [&]() {
            switch (t) {
            using enum tt::unicode_break_opportunity;
            case no: return "X";
            case yes: return ":";
            case mandatory: return "!";
            case unassigned: return "-";
            default: tt_no_default();
            }
        }();
        return std::formatter<char const *, CharT>::format(s, fc);
    }
};
