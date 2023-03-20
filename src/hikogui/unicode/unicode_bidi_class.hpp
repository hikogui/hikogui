// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>
#include <string_view>
#include "ucd_bidi_classes.hpp"
#include "../utility/module.hpp"

namespace hi::inline v1 {

[[nodiscard]] constexpr bool is_isolate_starter(unicode_bidi_class const &rhs) noexcept
{
    using enum unicode_bidi_class;
    return rhs == LRI || rhs == RLI || rhs == FSI;
}

[[nodiscard]] constexpr bool is_isolate_formatter(unicode_bidi_class const &rhs) noexcept
{
    using enum unicode_bidi_class;
    return is_isolate_starter(rhs) || rhs == PDI;
}

[[nodiscard]] constexpr bool is_NI(unicode_bidi_class const &rhs) noexcept
{
    using enum unicode_bidi_class;
    return rhs == B || rhs == S || rhs == WS || rhs == ON || rhs == FSI || rhs == LRI || rhs == RLI || rhs == PDI;
}

[[nodiscard]] constexpr bool is_control(unicode_bidi_class const &rhs) noexcept
{
    using enum unicode_bidi_class;
    return rhs == RLE or rhs == LRE or rhs == RLO or rhs == LRO or rhs == PDF or rhs == BN;
}

[[nodiscard]] constexpr unicode_bidi_class unicode_bidi_class_from_string(std::string_view str) noexcept
{
    using enum unicode_bidi_class;

    if (str == "L") {
        return L;
    } else if (str == "R") {
        return R;
    } else if (str == "AL") {
        return AL;
    } else if (str == "EN") {
        return EN;
    } else if (str == "ES") {
        return ES;
    } else if (str == "ET") {
        return ET;
    } else if (str == "AN") {
        return AN;
    } else if (str == "CS") {
        return CS;
    } else if (str == "NSM") {
        return NSM;
    } else if (str == "BN") {
        return BN;
    } else if (str == "B") {
        return B;
    } else if (str == "S") {
        return S;
    } else if (str == "WS") {
        return WS;
    } else if (str == "ON") {
        return ON;
    } else if (str == "LRE") {
        return LRE;
    } else if (str == "LRO") {
        return LRO;
    } else if (str == "RLE") {
        return RLE;
    } else if (str == "RLO") {
        return RLO;
    } else if (str == "PDF") {
        return PDF;
    } else if (str == "LRI") {
        return LRI;
    } else if (str == "RLI") {
        return RLI;
    } else if (str == "FSI") {
        return FSI;
    } else if (str == "PDI") {
        return PDI;
    } else {
        hi_no_default();
    }
}

} // namespace hi::inline v1
