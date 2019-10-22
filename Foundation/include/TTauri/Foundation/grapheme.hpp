// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include "TTauri/Foundation/globals.hpp"

namespace TTauri {

/*! A grapheme, what a user thinks a character is.
 * This will exclude ligatures, because a user would see those as separate characters.
 */
class grapheme {
    /*! This value contains up to 3 code-points, or a pointer+length to an array
     * of code-points located on the heap.
     *
     * If bit 0 is '1' the value contains up to 3 code-points as follows:
     *    - 63:43   3rd code-point, or zero
     *    - 42:22   2nd code-point, or zero
     *    - 21:1    1st code-point, or zero
     *    - 0       '1'
     *
     * if bit 0 is '0' the value contains a length+pointer as follows:
     *    - 63:48   Length
     *    - 47:0    Pointer to char32_t array on the heap;
     *              bottom two bits are zero, due to alignment.
     */
    uint64_t value;

public:
    force_inline grapheme() noexcept : value(1) {}

    force_inline ~grapheme() {
        delete_pointer();
    }

    force_inline grapheme(const grapheme& other) noexcept {
        value = other.value;
        if (other.has_pointer()) {
            value = create_pointer(other.get_pointer(), other.size());
        }
    }

    force_inline grapheme& operator=(const grapheme& other) noexcept {
        delete_pointer();
        value = other.value;
        if (other.has_pointer()) {
            value = create_pointer(other.get_pointer(), other.size());
        }
        return *this;
    }

    force_inline grapheme(grapheme&& other) noexcept {
        value = other.value;
        other.value = 1;
    }

    force_inline grapheme& operator=(grapheme&& other) noexcept {
        delete_pointer();
        value = other.value;
        other.value = 1;
        return *this;
    }

    force_inline explicit grapheme(std::u32string_view codePoints) noexcept :
        value(0)
    {
        let codePointsNFC = Foundation_globals->unicodeData->toNFC(codePoints);

        switch (codePointsNFC.size()) {
        case 3:
            value |= (static_cast<uint64_t>(codePointsNFC[2] & 0x1f'ffff) << 43);
            [[fallthrough]];
        case 2:
            value |= (static_cast<uint64_t>(codePointsNFC[1] & 0x1f'ffff) << 22);
            [[fallthrough]];
        case 1:
            value |= (static_cast<uint64_t>(codePointsNFC[0] & 0x1f'ffff) << 1);
            [[fallthrough]];
        case 0:
            value |= 1;
            break;
        default:
            if (codePointsNFC.size() <= std::numeric_limits<uint16_t>::max()) {
                value = create_pointer(codePointsNFC.data(), codePointsNFC.size());
            } else {
                value = (0x00'fffdULL << 43) | 1; // Replacement character.
            }
        }
    }

    force_inline explicit grapheme(char32_t codePoint) noexcept :
        grapheme(std::u32string_view{&codePoint, 1}) {}

    explicit operator std::u32string () const noexcept {
        if (has_pointer()) {
            return {get_pointer(), size()};
        } else {
            auto r = std::u32string{};
            auto tmp = value >> 1;
            for (size_t i = 0; i < 3; i++, tmp >>= 21) {
                if (auto codePoint = static_cast<char32_t>(tmp & 0x1f'ffff)) {
                    r += codePoint;
                } else {
                    return r;
                }
            }
            return r;
        }
    }

    force_inline size_t size() const noexcept {
        if (has_pointer()) {
            return value >> 48;
        } else {
            auto tmp = value >> 1;
            size_t i;
            for (i = 0; i < 3; i++, tmp >>= 21) {
                if ((tmp & 0x1f'ffff) == 0) {
                    return i;
                }
            }
            return i;
        }
    }

    force_inline std::u32string NFC() const noexcept {
        return static_cast<std::u32string>(*this);
    }

    force_inline std::u32string NFD() const noexcept {
        return Foundation_globals->unicodeData->toNFD(static_cast<std::u32string>(*this));
    }

    force_inline std::u32string NFKC() const noexcept {
        return Foundation_globals->unicodeData->toNFKC(static_cast<std::u32string>(*this));
    }

    force_inline std::u32string NFKD() const noexcept {
        return Foundation_globals->unicodeData->toNFKD(static_cast<std::u32string>(*this));
    }

private:
    force_inline bool has_pointer() const noexcept {
        return (value & 1) == 0;
    }

    static uint64_t create_pointer(char32_t const *data, size_t size) noexcept {
        auto ptr = new char32_t [size];
        std::memcpy(ptr, data, size);

        auto iptr = reinterpret_cast<ptrdiff_t>(ptr);
        auto uptr = static_cast<uint64_t>(iptr << 16) >> 16;
        return (size << 48) | uptr;
    }

    force_inline char32_t *get_pointer() const noexcept {
        auto uptr = (value << 16);
        auto iptr = static_cast<ptrdiff_t>(uptr) >> 16;
        return std::launder(reinterpret_cast<char32_t *>(iptr));
    }

    force_inline void delete_pointer() noexcept {
        if (has_pointer()) {
            delete [] get_pointer();
        }
    }
};

inline bool operator<(grapheme const& a, grapheme const& b) noexcept {
    return a.NFKC() < b.NFKC();
}

inline bool operator==(grapheme const& a, grapheme const& b) noexcept {
    return a.NFKC() == b.NFKC();
}




}
