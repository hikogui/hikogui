// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Grapheme.hpp"
#include <cstdint>

namespace tt {

/** Unicode Ranges based on the OS/2 table in TrueType fonts.
*/
struct UnicodeRanges {
    uint32_t value[4];

    UnicodeRanges() noexcept {
        value[0] = 0;
        value[1] = 0;
        value[2] = 0;
        value[3] = 0;
    }

    UnicodeRanges(char32_t c) noexcept : UnicodeRanges() {
        add(c);
    }

    UnicodeRanges(Grapheme g) noexcept : UnicodeRanges() {
        for (ssize_t i = 0; i != nonstd::ssize(g); ++i) {
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

    [[nodiscard]] bool contains(Grapheme g) const noexcept {
        for (ssize_t i = 0; i != nonstd::ssize(g); ++i) {
            if (!contains(g[i])) {
                return false;
            }
        }
        return true;
    }

    void set_bit(int i) noexcept {
        tt_assume(i >= 0 && i < 128);
        value[i / 32] |= static_cast<uint32_t>(1) << (i % 32);
    }

    bool get_bit(int i) const noexcept {
        tt_assume(i >= 0 && i < 128);
        return (value[i / 32] & static_cast<uint32_t>(1) << (i % 32)) != 0;
    }

    int popcount() const noexcept {
        int r = 0;
        for (int i = 0; i != 4; ++i) {
            r += ::tt::popcount(value[i]);
        }
        return r;
    }


    UnicodeRanges &operator|=(UnicodeRanges const &rhs) noexcept {
        for (int i = 0; i != 4; ++i) {
            value[i] |= rhs.value[i];
        }
        return *this;
    }

    [[nodiscard]] friend std::string to_string(UnicodeRanges const &rhs) noexcept {
        return fmt::format("{:08x}:{:08x}:{:08x}:{:08x}", rhs.value[3], rhs.value[2], rhs.value[1], rhs.value[0]);
    }

    /** The lhs has at least all bits on the rhs set.
    */
    [[nodiscard]] friend bool operator>=(UnicodeRanges const &lhs, UnicodeRanges const &rhs) noexcept {
        for (int i = 0; i < 4; i++) {
            if (!((lhs.value[i] & rhs.value[i]) == rhs.value[i])) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] friend UnicodeRanges operator|(UnicodeRanges const &lhs, UnicodeRanges const &rhs) noexcept {
        auto r = lhs;
        r |= rhs;
        return r;
    }

    friend std::ostream &operator<<(std::ostream &lhs, UnicodeRanges const &rhs) {
        return lhs << to_string(rhs);
    }
};

}