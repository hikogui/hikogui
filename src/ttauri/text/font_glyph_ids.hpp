// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "glyph_id.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../architecture.hpp"
#include <tuple>

namespace tt {
struct graphic_path;
class font;

// "Compatibility mappings are guaranteed to be no longer than 18 characters, although most consist of just a few characters."
// https://unicode.org/reports/tr44/ (TR44 5.7.3)
class font_glyph_ids_long {
    int8_t nr_glyphs = 0;
    std::array<glyph_id, 18> glyph_ids;

    font_glyph_ids_long() noexcept = delete;
    font_glyph_ids_long(font_glyph_ids_long const &rhs) noexcept = default;
    font_glyph_ids_long(font_glyph_ids_long &&rhs) noexcept = default;
    font_glyph_ids_long &operator=(font_glyph_ids_long const &rhs) noexcept = default;
    font_glyph_ids_long &operator=(font_glyph_ids_long &&rhs) noexcept = default;

    font_glyph_ids_long(glyph_id g1, glyph_id g2, glyph_id g3) noexcept
    {
        (*this) += g1;
        (*this) += g2;
        (*this) += g3;
    }

    font_glyph_ids_long operator+=(glyph_id rhs) noexcept
    {
        tt_axiom(nr_glyphs >= 0);
        tt_axiom(nr_glyphs < std::ssize(glyph_ids));
        glyph_ids[nr_glyphs++] = rhs;
        return *this;
    }

    [[nodiscard]] size_t hash() const noexcept
    {
        tt_axiom(nr_glyphs > 3);

        size_t r = 0;
        std::memcpy(&r, glyph_ids.data(), sizeof(r));
        r ^= nr_glyphs;
        return r;
    }

    [[nodiscard]] friend bool operator==(font_glyph_ids_long const &lhs, font_glyph_ids_long const &rhs) noexcept
    {
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
public:
    constexpr font_glyph_ids() noexcept : _font(nullptr)
    {
        _glyphs.ids_short = ids_short_empty;
    }

    constexpr font_glyph_ids(tt::font const &font) noexcept : _font(&font)
    {
        _glyphs.ids_short = ids_short_empty;
    }

    font_glyph_ids(font_glyph_ids const &other) noexcept : _font(other._font)
    {
        if (other.is_long()) {
            _glyphs.ids_long = new font_glyph_ids_long(*other._glyphs.ids_long);
        } else {
            _glyphs.ids_short = other._glyphs.ids_short;
        }
    }

    font_glyph_ids(font_glyph_ids &&other) noexcept : _font(other._font), _glyphs(other._glyphs)
    {
        other._glyphs.ids_short = ids_short_empty;
    }

    font_glyph_ids &operator=(font_glyph_ids const &other) noexcept
    {
        tt_return_on_self_assignment(other);

        if (is_long()) {
            delete _glyphs.ids_long;
        }

        _font = other._font;
        if (other.is_long()) {
            _glyphs.ids_long = new font_glyph_ids_long(*other._glyphs.ids_long);
        } else {
            _glyphs.ids_short = other._glyphs.ids_short;
        }

        return *this;
    }

    font_glyph_ids &operator=(font_glyph_ids &&other) noexcept
    {
        using std::swap;

        swap(_font, other._font);
        swap(_glyphs, other._glyphs);
        return *this;
    }

    ~font_glyph_ids() noexcept
    {
        if (is_long()) {
            delete _glyphs.ids_long;
        }
    }

    void clear() noexcept
    {
        if (is_long()) {
            delete _glyphs.ids_long;
        }
        _glyphs.ids_short = ids_short_empty;
    }

    operator bool() const noexcept
    {
        return size() > 0;
    }

    [[nodiscard]] font const &font() const noexcept
    {
        tt_axiom(_font);
        return *_font;
    }

    void set_font(tt::font const &font) noexcept
    {
        _font = &font;
    }

    font_glyph_ids &operator+=(glyph_id rhs) noexcept
    {
        switch (size()) {
        case 0: _glyphs.ids_short = (_glyphs.ids_short & 0xffff'ffff'0000'0000) | (uint64_t{rhs} << 16) | 1; break;
        case 1: _glyphs.ids_short = (_glyphs.ids_short & 0xffff'0000'ffff'0000) | (uint64_t{rhs} << 32) | 2; break;
        case 2: _glyphs.ids_short = (_glyphs.ids_short & 0x0000'ffff'ffff'0000) | (uint64_t{rhs} << 48) | 3; break;
        case 3: _glyphs.ids_long = new font_glyph_ids_long((*this)[0], (*this)[1], (*this)[2]); [[fallthrough]];
        default: *_glyphs.ids_long += rhs;
        }
        return *this;
    }

    [[nodiscard]] glyph_id front() const noexcept
    {
        if (size() == 0) {
            return glyph_id{};
        } else {
            return (*this)[0];
        }
    }

    [[nodiscard]] glyph_id operator[](size_t index) const noexcept
    {
        tt_axiom(index < size());
        if (is_long()) {
            tt_axiom(index < 18);
            return _glyphs.ids_long->glyph_ids[index];
        } else {
            switch (index) {
            case 0: return glyph_id{(_glyphs.ids_short >> 16) & glyph_id::mask};
            case 1: return glyph_id{(_glyphs.ids_short >> 32) & glyph_id::mask};
            case 2: return glyph_id{(_glyphs.ids_short >> 48) & glyph_id::mask};
            default: tt_no_default();
            }
        }
    }

    [[nodiscard]] size_t size() const noexcept
    {
        if (is_long()) {
            return _glyphs.ids_long->nr_glyphs;
        } else if ((_glyphs.ids_short & 7) == 3) {
            return 3;
        } else if ((_glyphs.ids_short & 7) == 2) {
            return 2;
        } else if ((_glyphs.ids_short & 7) == 1) {
            return 1;
        } else {
            return 0;
        }
    }

    [[nodiscard]] size_t hash() const noexcept
    {
        auto r = is_long() ? _glyphs.ids_long->hash() : static_cast<size_t>(_glyphs.ids_short);
        r ^= static_cast<size_t>(reinterpret_cast<ptrdiff_t>(_font)) >> 3;
        return r;
    }

    [[nodiscard]] std::pair<graphic_path, aarectangle> get_path_and_bounding_box() const noexcept;
    [[nodiscard]] aarectangle get_bounding_box() const noexcept;

    [[nodiscard]] friend bool operator==(font_glyph_ids const &lhs, font_glyph_ids const &rhs) noexcept
    {
        // This function is written to expect the comparison to yield true. As it would
        // most likely be used inside a hash table.
        auto same = lhs._font == rhs._font;
        same &= lhs.is_long() == rhs.is_long();
        if (not lhs.is_long()) {
            [[likely]] same &= lhs._glyphs.ids_short == rhs._glyphs.ids_short;
        } else {
            same &= *lhs._glyphs.ids_long == *rhs._glyphs.ids_long;
        }
        return same;
    }

private:
    static constexpr uint64_t ids_short_empty = 4;

    tt::font const *_font;

    union {
        font_glyph_ids_long *ids_long;

        static_assert(sizeof(ids_long) == 8);

        /** A list of up to 3 glyphs.
         *
         * [15:0] num_short_glyphs, 0 = long glyph, 4 = empty
         * [31:16] glyph 0
         * [47:32] glyph 1
         * [63:48] glyph 2
         */
        uint64_t ids_short;
    } _glyphs;

    [[nodiscard]] bool is_long() const noexcept
    {
        uint64_t tmp = std::bit_cast<uint64_t>(_glyphs);
        // Pointers have the bottom 3 bits zero.
        return (tmp & 0x7) == 0;
    }
};

} // namespace tt

namespace std {

template<>
struct hash<tt::font_glyph_ids> {
    [[nodiscard]] size_t operator()(tt::font_glyph_ids const &rhs) const noexcept
    {
        return rhs.hash();
    }
};

} // namespace std
