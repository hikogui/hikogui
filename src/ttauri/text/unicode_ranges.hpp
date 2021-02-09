// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "grapheme.hpp"
#include <cstdint>
#include <bit>

namespace tt {

/** Unicode Ranges based on the OS/2 table in TrueType fonts.
*/
struct unicode_ranges {
    uint32_t value[4];

    unicode_ranges() noexcept {
        value[0] = 0;
        value[1] = 0;
        value[2] = 0;
        value[3] = 0;
    }

    unicode_ranges(char32_t c) noexcept : unicode_ranges() {
        add(c);
    }

    unicode_ranges(grapheme g) noexcept : unicode_ranges() {
        for (ssize_t i = 0; i != std::ssize(g); ++i) {
            add(g[i]);
        }
    }

    operator bool () const noexcept {
        return (value[0] != 0) || (value[1] != 0) || (value[2] != 0) || (value[3] != 0);
    }

    /** Add code point to unicode-ranges.
    */
    void add(char32_t c) noexcept;

    /** Add code points to unicode-ranges.
    * @param first First code point.
    * @param last One beyond the last code point.
    */
    void add(char32_t first, char32_t last) noexcept;

    /** Check if the code point is present in the unicode-ranges.
    */
    [[nodiscard]] bool contains(char32_t c) const noexcept;

    [[nodiscard]] bool contains(grapheme g) const noexcept {
        for (ssize_t i = 0; i != std::ssize(g); ++i) {
            if (!contains(g[i])) {
                return false;
            }
        }
        return true;
    }

    void set_bit(int i) noexcept {
        tt_axiom(i >= 0 && i < 128);
        value[i / 32] |= static_cast<uint32_t>(1) << (i % 32);
    }

    bool get_bit(int i) const noexcept {
        tt_axiom(i >= 0 && i < 128);
        return (value[i / 32] & static_cast<uint32_t>(1) << (i % 32)) != 0;
    }

    int popcount() const noexcept {
        int r = 0;
        for (int i = 0; i != 4; ++i) {
            r += std::popcount(value[i]);
        }
        return r;
    }


    unicode_ranges &operator|=(unicode_ranges const &rhs) noexcept {
        for (int i = 0; i != 4; ++i) {
            value[i] |= rhs.value[i];
        }
        return *this;
    }

    [[nodiscard]] friend std::string to_string(unicode_ranges const &rhs) noexcept {
        return fmt::format("{:08x}:{:08x}:{:08x}:{:08x}", rhs.value[3], rhs.value[2], rhs.value[1], rhs.value[0]);
    }

    /** The lhs has at least all bits on the rhs set.
    */
    [[nodiscard]] friend bool operator>=(unicode_ranges const &lhs, unicode_ranges const &rhs) noexcept {
        for (int i = 0; i < 4; i++) {
            if (!((lhs.value[i] & rhs.value[i]) == rhs.value[i])) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] friend unicode_ranges operator|(unicode_ranges const &lhs, unicode_ranges const &rhs) noexcept {
        auto r = lhs;
        r |= rhs;
        return r;
    }

    friend std::ostream &operator<<(std::ostream &lhs, unicode_ranges const &rhs) {
        return lhs << to_string(rhs);
    }
};

}