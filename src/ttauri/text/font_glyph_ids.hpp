// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "glyph_id.hpp"
#include "font_id.hpp"
#include "../hash.hpp"
#include "../tagged_id.hpp"
#include "../aarect.hpp"
#include <tuple>

namespace tt {
struct graphic_path;
}

namespace tt {

// "Compatibility mappings are guaranteed to be no longer than 18 characters, although most consist of just a few characters."
// https://unicode.org/reports/tr44/ (TR44 5.7.3)
class font_glyph_ids_long {
    int8_t nr_glyphs = 0;
    std::array<glyph_id,18> glyph_ids;

    font_glyph_ids_long() noexcept = delete;
    font_glyph_ids_long(font_glyph_ids_long const &rhs) noexcept = default;
    font_glyph_ids_long(font_glyph_ids_long &&rhs) noexcept = default;
    font_glyph_ids_long &operator=(font_glyph_ids_long const &rhs) noexcept = default;
    font_glyph_ids_long &operator=(font_glyph_ids_long &&rhs) noexcept = default;

    font_glyph_ids_long(glyph_id g1, glyph_id g2, glyph_id g3) noexcept {
        (*this) += g1;
        (*this) += g2;
        (*this) += g3;
    }

    font_glyph_ids_long operator+=(glyph_id rhs) noexcept {
        tt_axiom(nr_glyphs >= 0);
        tt_axiom(nr_glyphs < std::ssize(glyph_ids));
        glyph_ids[nr_glyphs++] = rhs;
        return *this;
    }

    [[nodiscard]] size_t hash() const noexcept {
        tt_axiom(nr_glyphs > 3);
        tt_axiom(nr_glyphs < std::ssize(glyph_ids));

        uint64_t r = 0;
        for (int8_t i = 0; i != nr_glyphs; ++i) {
            r = hash_mix_two(r, std::hash<glyph_id>{}(glyph_ids[i]));
        }

        return r;
    }

    [[nodiscard]] friend bool operator==(font_glyph_ids_long const &lhs, font_glyph_ids_long const &rhs) noexcept {
        tt_axiom(lhs.nr_glyphs > 3);
        tt_axiom(rhs.nr_glyphs > 3);
        tt_axiom(lhs.nr_glyphs < std::ssize(lhs.glyph_ids));
        tt_axiom(rhs.nr_glyphs < std::ssize(rhs.glyph_ids));

        if (lhs.nr_glyphs == rhs.nr_glyphs) {
            for (int8_t i = 0; i != lhs.nr_glyphs; ++i) {
                if (lhs.glyph_ids[i] != rhs.glyph_ids[i]) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    }

    friend class font_glyph_ids;
};

class font_glyph_ids {
    constexpr static uint64_t empty = 0xffff'ffff'ffff'ffff;

    /*
    * 0 to 3 glyph_ids, with or without a font_id.
    * 63:48 glyph_id[2]
    * 47:32 glyph_id[1]
    * 31:16 glyph_id[0]
    *    15 '1'
    * 14: 0 font_id
    *
    * More than 3 glyphs
    * 63:16 fontGlyphsIDs_long *
    *    15 '0'
    * 14: 0 font_id
    */
    uint64_t value;

public:
    font_glyph_ids() noexcept : value(empty) {}

    font_glyph_ids(font_glyph_ids const &rhs) noexcept : value(rhs.value) {
        if (rhs.has_pointer()) {
            value = new_pointer(*(rhs.get_pointer()));
        }
    }

    font_glyph_ids(font_glyph_ids &&rhs) noexcept : value(rhs.value) {
        rhs.value = empty;
    }

    font_glyph_ids &operator=(font_glyph_ids const &rhs) noexcept {
        if (this != &rhs) {
            delete_pointer();
            value = rhs.value;
            if (rhs.has_pointer()) {
                value = new_pointer(*(rhs.get_pointer()));
            }
        }
        return *this;
    }

    font_glyph_ids &operator=(font_glyph_ids &&rhs) noexcept {
        if (this != &rhs) {
            using std::swap;
            swap(value, rhs.value);
        }
        return *this;
    }

    ~font_glyph_ids() noexcept {
        delete_pointer();
    }

    void clear() noexcept {
        delete_pointer();
        value = empty;
    }

    operator bool () const noexcept {
        return size() > 0;
    }

    [[nodiscard]] tt::font_id font_id() const noexcept
    {
        return tt::font_id{value & font_id::mask};
    }

    void set_font_id(tt::font_id font_id) noexcept
    {
        value = (value & ~static_cast<uint64_t>(font_id::mask)) | static_cast<uint64_t>(font_id);
    }

    font_glyph_ids &operator+=(glyph_id rhs) noexcept {
        switch (size()) {
        case 0: value = (value & 0xffff'ffff'0000'ffff) | (static_cast<uint64_t>(rhs) << 16); break;
        case 1: value = (value & 0xffff'0000'ffff'ffff) | (static_cast<uint64_t>(rhs) << 32); break;
        case 2: value = (value & 0x0000'ffff'ffff'ffff) | (static_cast<uint64_t>(rhs) << 48); break;
        case 3:
            value = (value & font_id::mask) | new_pointer((*this)[0], (*this)[1], (*this)[2]);
            [[fallthrough]];
        default:
            *(get_pointer()) += rhs;
        }
        return *this;
    }

    [[nodiscard]] glyph_id front() const noexcept {
        if (size() == 0) {
            return glyph_id{};
        } else {
            return (*this)[0];
        }
    }

    [[nodiscard]] glyph_id operator[](size_t index) const noexcept {
        if (has_pointer()) {
            tt_axiom(index < 18);
            return get_pointer()->glyph_ids[index];
        } else {
            switch (index) {
            case 0: return glyph_id{(value >> 16) & glyph_id::mask};
            case 1: return glyph_id{(value >> 32) & glyph_id::mask};
            case 2: return glyph_id{(value >> 48) & glyph_id::mask};
            default: tt_no_default();
            }
        }
    }

    [[nodiscard]] size_t size() const noexcept {
        if (has_pointer()) {
            return get_pointer()->nr_glyphs;
        } else if (!(*this)[0]) {
            return 0;
        } else if (!(*this)[1]) {
            return 1;
        } else if (!(*this)[2]) {
            return 2;
        } else {
            return 3;
        }
    }

    [[nodiscard]] size_t hash() const noexcept {
        if (has_pointer()) {
            return get_pointer()->hash();
        } else {
            return std::hash<uint64_t>{}(value);
        }
    }

    [[nodiscard]] std::pair<graphic_path,aarect> getPathAndBoundingBox() const noexcept;
    [[nodiscard]] aarect getBoundingBox() const noexcept;

private:
    [[nodiscard]] bool has_pointer() const noexcept {
        return (value & 0x8000) == 0;
    }

    [[nodiscard]] font_glyph_ids_long const *get_pointer() const noexcept {
        tt_axiom(has_pointer());
        return std::launder(reinterpret_cast<font_glyph_ids_long const *>(static_cast<ptrdiff_t>(value) >> 16));
    }

    [[nodiscard]] font_glyph_ids_long *get_pointer() noexcept {
        tt_axiom(has_pointer());
        return std::launder(reinterpret_cast<font_glyph_ids_long *>(static_cast<ptrdiff_t>(value) >> 16));
    }

    void delete_pointer() noexcept {
        if (has_pointer()) {
            delete get_pointer();
            value = empty;
        }
    }

    template<typename... Args>
    [[nodiscard]] static uint64_t new_pointer(Args &&... args) noexcept {
        auto *ptr = new font_glyph_ids_long(std::forward<Args>(args)...);
        return static_cast<uint64_t>(reinterpret_cast<ptrdiff_t>(ptr) << 16);
    }

public:
    [[nodiscard]] friend bool operator==(font_glyph_ids const &lhs, font_glyph_ids const &rhs) noexcept {
        if (lhs.has_pointer() == rhs.has_pointer()) {
            if (lhs.has_pointer()) {
                return *(lhs.get_pointer()) == *(rhs.get_pointer());
            } else {
                return lhs.value == rhs.value;
            }
        } else {
            tt_axiom(lhs.size() != rhs.size());
            return false;
        }
    }
};

}

namespace std {

template<>
struct hash<tt::font_glyph_ids> {
    [[nodiscard]] size_t operator()(tt::font_glyph_ids const &rhs) const noexcept {
        return rhs.hash();
    }
};

}
