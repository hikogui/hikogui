// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "glyph_id.hpp"
#include "font.hpp"
#include "glyph_atlas_info.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../required.hpp"
#include <bit>
#include <memory>
#include <cstddef>
#include <array>
#include <cstdint>
#include <functional>

namespace tt::inline v1 {
namespace detail {

class glyph_ids_long {
public:
    constexpr glyph_ids_long(glyph_ids_long const &) noexcept = default;
    constexpr glyph_ids_long(glyph_ids_long &&) noexcept = default;
    constexpr glyph_ids_long &operator=(glyph_ids_long const &) noexcept = default;
    constexpr glyph_ids_long &operator=(glyph_ids_long &&) noexcept = default;

    /** Construct a list of glyphs starting with a packed set of glyphs.
     *
     * @param value The value from the `glyph_ids` internal value,
     * @param new_id The new id being added to glyph_ids.
     */
    constexpr glyph_ids_long(size_t value, glyph_id new_id) noexcept : _glyphs(), _size(sizeof(size_t) == 4 ? 2 : 4)
    {
        if constexpr (sizeof(size_t) == 4) {
            std::get<0>(_glyphs) = glyph_id{(value >> 16) & 0xffff};
            std::get<1>(_glyphs) = *new_id;
        } else {
            std::get<0>(_glyphs) = glyph_id{(value >> 16) & 0xffff};
            std::get<1>(_glyphs) = glyph_id{(value >> 32) & 0xffff};
            std::get<2>(_glyphs) = glyph_id{(value >> 48) & 0xffff};
            std::get<3>(_glyphs) = *new_id;
        }
    }

    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return _size;
    }

    [[nodiscard]] constexpr size_t hash() const noexcept
    {
        size_t r = 0;

        for (auto i = 0_uz; i != _size; ++i) {
            r ^= _glyphs[i];
            r ^= r << 16;
            r ^= r >> 16;
        }

        return r;
    }

    constexpr glyph_ids_long &operator+=(glyph_id id) noexcept
    {
        // On overflow silently drop glyphs.
        if (_size < _glyphs.size()) {
            _glyphs[_size++] = id;
        }
        return *this;
    }

    [[nodiscard]] constexpr glyph_id const &operator[](size_t i) const noexcept
    {
        return _glyphs[i];
    }

    [[nodiscard]] constexpr friend bool operator==(glyph_ids_long const &, glyph_ids_long const &) noexcept = default;

private:
    // "Compatibility mappings are guaranteed to be no longer than 18 characters, although most consist of just a few characters."
    // https://unicode.org/reports/tr44/ (TR44 5.7.3)
    std::array<glyph_id, 18> _glyphs;
    uint8_t _size;
};
}

/** Glyph IDs of a single grapheme.
 *
 * This class holds a set of glyphs belonging to a single grapheme.
 * All glyph ids are 16-bit integers that belong to a single font.
 *
 * The maximum number of glyphs are based on:
 *    "Compatibility mappings are guaranteed to be no longer than 18 characters, although most consist of just a few characters."
 *    https://unicode.org/reports/tr44/ (TR44 5.7.3)
 *
 */
class glyph_ids {
public:
    constexpr ~glyph_ids()
    {
        if (is_long()) {
            delete _ptr;
        }
    }

    constexpr glyph_ids(glyph_ids const &other) noexcept : _font(other._font), _ptr(other._ptr)
    {
        if (is_long()) {
            _ptr = new detail::glyph_ids_long(*_ptr);
        }
    }

    constexpr glyph_ids &operator=(glyph_ids const &other) noexcept
    {
        tt_return_on_self_assignment(other);

        _font = other._font;
        if (is_long()) {
            delete _ptr;
        }
        _ptr = other._ptr;
        if (is_long()) {
            _ptr = new detail::glyph_ids_long(*_ptr);
        }
        return *this;
    }

    constexpr glyph_ids(glyph_ids &&other) noexcept : _font(other._font), _ptr(std::exchange(other._ptr, make_ptr(1))) {}

    constexpr glyph_ids &operator=(glyph_ids &&other) noexcept
    {
        _font = other._font;
        std::swap(_ptr, other._ptr);
        return *this;
    }

    /** Create an empty glyph_ids object.
     *
     * This consructor should only be used for default member initialization.
     * Normally the font should always be assigned.
     */
    constexpr glyph_ids() noexcept : _font(nullptr), _ptr(make_ptr(1)) {}

    /** Create an empty glyph_ids for a font.
     *
     * This is the normal constructor.
     *
     * @param font The font to be used for this glyph_ids.
     */
    constexpr glyph_ids(tt::font const &font) noexcept : _font(*font), _ptr(make_ptr(1)) {}

    /** Get the font for this glyph_ids object.
     */
    [[nodiscard]] constexpr font const &font() const noexcept
    {
        tt_axiom(_font);
        return *_font;
    }

    /** Set the font for this glyph_ids object.
     */
    //void set_font(tt::font const &font) noexcept
    //{
    //    _font = &font;
    //}

    /** Clear the glyphs in this glyph_ids object.
     *
     * The font remains attached to this object.
     */
    constexpr void clear() noexcept
    {
        if (is_long()) {
            delete _ptr;
        }
        _ptr = make_ptr(1);
    }

    /** Check if glyphs are attached.
     */
    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _ptr == make_ptr(1);
    }

    /** Check if glyphs are attached.
     */
    constexpr operator bool() const noexcept
    {
        return not enpty();
    }

    /** Check if this object contains a single glyph.
     */
    [[nodiscard]] constexpr bool is_single() const noexcept
    {
        return _ptr == make_ptr(3);
    }

    /** Get the value of the single glyph.
     */
    [[nodiscard]] constexpr glyph_id get_single() const noexcept
    {
        tt_axiom(is_single());
        return get_glyph(1);
    }

    /** Get the number of glyphs.
     */
    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return is_long() ? _ptr->size() : (make_value(_ptr) >> 1) & 7;
    }

    /** Get the hash value.
     */
    [[nodiscard]] constexpr size_t hash() const noexcept
    {
        return is_long() ? _ptr->hash() : make_value(_ptr);
    }

    /** Add a glyph to this object.
     *
     * @param id The glyph to add.
     */
    constexpr glyph_ids &operator+=(glyph_id id) noexcept
    {
        constexpr size_t max_short_size = sizeof(_ptr) == 4 ? 7 : 15;

        if (is_long()) {
            *_ptr += id;

        } else if (short_size() < max_short_size) {
            ttlet new_index = increment_size();
            set_glyph(index, id);

        } else {
            _ptr = new detail::glyph_ids_long(value, id);
        }
        return *this;
    }

    /** Get a glyph.
     *
     * @param index The index to the glyph to access
     */
    [[nodiscard]] constexpr glyph_id operator[](size_t i) const noexcept
    {
        tt_axiom(i < size());

        if (is_long()) {
            return (*_ptr)[i];
        } else {
            return get_glyph(i);
        }
    }

    [[nodiscard]] constexpr friend bool operator==(glyph_ids const &lhs, glyph_ids const &rhs) noexcept
    {
        ttlet lhs_value = make_value(lhs._ptr);
        ttlet rhs_value = make_value(rhs._ptr);

        if (lhs._font != rhs._font) {
            return false;
        } else if (lhs_value == rhs_value) {
            return true;
        } else {
            return (lhs_value | rhs_value) & 1 == 0 and *lhs._ptr == *rhs._ptr;
        }
    }

    [[nodiscard]] std::pair<graphic_path, aarectangle> get_path_and_bounding_box() const noexcept;

    /** Get the bounding box of the combined glyphs.
     *
     * The 
     */
    [[nodiscard]] aarectangle get_bounding_box() const noexcept;

private:
    tt::font const *_font;
    detail::glyph_ids_long *_ptr;

    [[nodiscard]] constexpr size_t increment_size() const noexcept
    {
        tt_axiom(is_short());

        ttlet new_value = make_value(_ptr) + 2; 
        _ptr = make_ptr(new_value);
        return (new_value >> 1) & 15;
    }

    [[nodiscard]] constexpr glyph_id get_glyph(size_t i) const noexcept
    {
        tt_axiom(is_short());

        auto shift = (i + 1) * 16;
        return glyph_id{(make_value(_ptr) >> shift) & 0xffff};
    }

    constexpr void set_glyph(size_t i, glyph_id id) noexcept
    {
        tt_axiom(is_short());

        auto shift = (i + 1) * 16;
        auto mask = size_t{0xffff} << shift;
        _ptr = make_ptr((make_value(_ptr) & ~mask) | (*id << shift));
    }

    [[nodiscard]] constexpr size_t short_size() const noexcept
    {
        tt_axiom(is_short());
        return (make_value(_ptr) >> 1) & 7;
    }

    [[nodiscard]] constexpr bool is_short() const noexcept
    {
        return static_cast<bool>(make_value(_ptr) & 1);
    }

    [[nodiscard]] constexpr bool is_long() const noexcept
    {
        return not is_short();
    }

    [[nodiscard]] static constexpr detail::glyph_ids_long *make_ptr(size_t value) noexcept
    {
        return std::bit_cast<detail::glyph_ids_long *>(value);
    }

    [[nodiscard]] static constexpr size_t make_value(detail::glyph_ids_long *ptr) noexcept
    {
        return std::bit_cast<size_t>(ptr);
    }
};

} // namespace tt::inline v1

template<>
struct std::hash<tt::glyph_ids> {
    [[nodiscard]] constexpr size_t operator()(tt::glyph_ids const &rhs) const noexcept
    {
        return rhs.hash();
    }
};
