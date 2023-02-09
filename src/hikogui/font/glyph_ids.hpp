// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "glyph_id.hpp"
#include "glyph_atlas_info.hpp"
#include "../graphic_path.hpp"
#include "../geometry/module.hpp"
#include "../utility/module.hpp"
#include "../lean_vector.hpp"
#include <bit>
#include <memory>
#include <cstddef>
#include <array>
#include <cstdint>
#include <functional>

hi_warning_push();
// C26401: Do not delete a raw pointer that is not an owner<T> (i.11).
// False positive, ~glyph_ids::glyph_ids() is owner of glyphs_ids::_ptr.
hi_warning_ignore_msvc(26401);
// C26409: Avoid calling new and delete explicitly, use std::make_unique<T> instead (r.11).
// glyph_ids implements a container.
hi_warning_ignore_msvc(26409);

namespace hi::inline v1 {
class font;

namespace detail {

class glyph_ids_long {
public:
    constexpr glyph_ids_long(glyph_ids_long const&) noexcept = default;
    constexpr glyph_ids_long(glyph_ids_long&&) noexcept = default;
    constexpr glyph_ids_long& operator=(glyph_ids_long const&) noexcept = default;
    constexpr glyph_ids_long& operator=(glyph_ids_long&&) noexcept = default;

    /** Construct a list of glyphs starting with a packed set of glyphs.
     *
     * @param value The value from the `glyph_ids` internal value,
     * @param new_id The new id being added to glyph_ids.
     */
    constexpr glyph_ids_long(std::size_t value, glyph_id new_id) noexcept : _num_glyphs(), _num_graphemes(), _glyphs()
    {
        if constexpr (sizeof(std::size_t) == 4) {
            _num_glyphs = 2;
            _num_graphemes = (value >> 4) & 0xf;
            std::get<0>(_glyphs) = glyph_id{(value >> 16) & 0xffff};
            std::get<1>(_glyphs) = new_id;
        } else {
            _num_glyphs = 4;
            _num_graphemes = (value >> 4) & 0xf;
            std::get<0>(_glyphs) = glyph_id{(value >> 16) & 0xffff};
            std::get<1>(_glyphs) = glyph_id{(value >> 32) & 0xffff};
            std::get<2>(_glyphs) = glyph_id{(value >> 48) & 0xffff};
            std::get<3>(_glyphs) = new_id;
        }
    }

    [[nodiscard]] constexpr std::size_t num_glyphs() const noexcept
    {
        return _num_glyphs;
    }

    [[nodiscard]] constexpr std::size_t num_graphemes() const noexcept
    {
        return _num_graphemes;
    }

    constexpr void set_num_graphemes(std::size_t num_graphemes) noexcept
    {
        hi_axiom(num_graphemes <= 0xf);
        _num_graphemes = narrow_cast<uint8_t>(num_graphemes);
    }

    [[nodiscard]] constexpr std::size_t hash() const noexcept
    {
        std::size_t r = 0;

        for (auto i = 0_uz; i != _num_glyphs; ++i) {
            r ^= *_glyphs[i];
            r ^= std::rotl(r, 16);
        }

        return r;
    }

    constexpr glyph_ids_long& operator+=(glyph_id id) noexcept
    {
        // On overflow silently drop glyphs.
        if (_num_glyphs < _glyphs.size()) {
            _glyphs[_num_glyphs++] = id;
        }
        return *this;
    }

    [[nodiscard]] constexpr glyph_id const& operator[](std::size_t i) const noexcept
    {
        hi_axiom(i < _num_glyphs);
        return _glyphs[i];
    }

    template<std::size_t I>
    [[nodiscard]] constexpr friend glyph_id get(glyph_ids_long const& rhs) noexcept
    {
        hi_axiom(I < rhs._num_glyphs);
        return std::get<I>(rhs._glyphs);
    }

    [[nodiscard]] constexpr friend bool operator==(glyph_ids_long const&, glyph_ids_long const&) noexcept = default;

private:
    uint8_t _num_glyphs;
    uint8_t _num_graphemes;

    // At least 18 glyphs to handle the maximum Unicode compatibility-decomposition.
    // Given a 64 byte allocation chunk, 16 bytes overhead, 2 bytes for the _num_graphemes
    // and _num_glyphs; we are left over with 46 bytes -> 23 glyphs.
    std::array<glyph_id, 23> _glyphs;
};

} // namespace detail

/** A set of glyph-ids of a font which composites into a single glyph.
 *
 * The normal operation for getting a glyph-ids is by:
 *  - By looking up a non-typographical-ligature grapheme in a font, returning 1 or more
 *    glyphs representing that single grapheme.
 *  - By morphing a sequence of glyph_ids objects into new glyph_ids objects, where some of
 *    the glyph_ids objects may get merged into a ligature of multiple graphemes.
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

    constexpr glyph_ids(glyph_ids const& other) noexcept : _font(other._font), _ptr(other._ptr)
    {
        if (is_long()) {
            _ptr = new detail::glyph_ids_long(*_ptr);
        }
    }

    constexpr glyph_ids& operator=(glyph_ids const& other) noexcept
    {
        hi_return_on_self_assignment(other);

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

    constexpr glyph_ids(glyph_ids&& other) noexcept : _font(other._font), _ptr(std::exchange(other._ptr, make_ptr(1))) {}

    constexpr glyph_ids& operator=(glyph_ids&& other) noexcept
    {
        _font = other._font;
        std::swap(_ptr, other._ptr);
        return *this;
    }

    /** Create an empty glyph_ids object.
     *
     * This constructor should only be used for default member initialization.
     * Normally the font should always be assigned.
     */
    constexpr glyph_ids() noexcept : _font(nullptr), _ptr(make_ptr(1)) {}

    /** Create an empty glyph_ids for a font.
     *
     * This is the normal constructor.
     *
     * @param font The font to be used for this glyph_ids.
     */
    constexpr glyph_ids(hi::font const& font) noexcept : _font(&font), _ptr(make_ptr(1)) {}

    /** Create a glyph_ids object from a font and a single glyph_id.
     */
    constexpr glyph_ids(hi::font const& font, hi::glyph_id glyph_id) noexcept : glyph_ids(font)
    {
        *this += glyph_id;
    }

    /** Create a glyph_ids object from a font and a list of glyph_ids.
     */
    glyph_ids(hi::font const& font, lean_vector<glyph_id> const& glyph_ids) noexcept : glyph_ids(font)
    {
        for (hilet glyph_id : glyph_ids) {
            *this += glyph_id;
        }
    }

    /** Get the font for this glyph_ids object.
     */
    [[nodiscard]] constexpr font const& font() const noexcept
    {
        hi_assert_not_null(_font);
        return *_font;
    }

    /** Set the font for this glyph_ids object.
     */
    void set_font(hi::font const& font) noexcept
    {
        _font = &font;
    }

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
        return not empty();
    }

    /** Check if this object contains a number of glyphs.
     *
     * A single glyph may represent 1 or more graphemes.
     *
     * @tparam N The number of glyphs to check, must fit in a short glyph.
     * @return True if the number of glyphs equals N.
     */
    template<std::size_t N>
    [[nodiscard]] constexpr bool has_num_glyphs() const noexcept
    {
        static_assert(N <= num_glyphs_mask);

        constexpr std::size_t mask = (num_glyphs_mask << num_glyphs_shift) | 1;
        constexpr std::size_t value = (N << num_glyphs_shift) | 1;
        return (make_value(_ptr) & mask) == value;
    }

    /** Get the value of the single glyph.
     */
    [[nodiscard]] constexpr glyph_id get_single() const noexcept
    {
        hi_axiom(has_num_glyphs<1>());
        return get_glyph(0);
    }

    /** Get the number of glyphs.
     */
    [[nodiscard]] constexpr std::size_t num_glyphs() const noexcept
    {
        return is_long() ? _ptr->num_glyphs() : (make_value(_ptr) >> 1) & num_glyphs_mask;
    }

    [[nodiscard]] constexpr std::size_t num_graphemes() const noexcept
    {
        return is_long() ? _ptr->num_graphemes() : (make_value(_ptr) >> 4) & 0xf;
    }

    constexpr void set_num_graphemes(std::size_t num_graphemes) noexcept
    {
        if (is_long()) {
            _ptr->set_num_graphemes(num_graphemes);
        } else {
            hi_axiom(num_graphemes <= num_graphemes_mask);
            _ptr = make_ptr(
                (make_value(_ptr) & ~(num_graphemes_mask << num_graphemes_shift)) | (num_graphemes << num_graphemes_shift));
        }
    }

    /** Get the hash value.
     */
    [[nodiscard]] constexpr std::size_t hash() const noexcept
    {
        return is_long() ? _ptr->hash() : make_value(_ptr);
    }

    /** Add a glyph to this object.
     *
     * @param id The glyph to add.
     */
    constexpr glyph_ids& operator+=(glyph_id id) noexcept
    {
        if (is_long()) {
            *_ptr += id;

        } else if (hilet index = short_num_glyphs(); index < num_glyphs_mask) {
            increment_num_glyphs();
            set_glyph(index, id);

        } else {
            _ptr = new detail::glyph_ids_long(make_value(_ptr), id);
        }
        return *this;
    }

    /** Get a glyph.
     *
     * @param index The index to the glyph to access
     */
    [[nodiscard]] constexpr glyph_id operator[](std::size_t index) const noexcept
    {
        hi_axiom(index < num_glyphs());

        if (is_long()) {
            return (*_ptr)[index];
        } else {
            return get_glyph(index);
        }
    }

    template<std::size_t I>
    [[nodiscard]] constexpr friend glyph_id get(glyph_ids const& rhs) noexcept
    {
        if (rhs.is_long()) {
            return get<I>(*rhs._ptr);
        } else {
            constexpr std::size_t shift = (I + 1) * 16;
            return glyph_id{(make_value(rhs._ptr) >> shift) & 0xffff};
        }
    }

    [[nodiscard]] constexpr friend bool operator==(glyph_ids const& lhs, glyph_ids const& rhs) noexcept
    {
        hilet lhs_value = make_value(lhs._ptr);
        hilet rhs_value = make_value(rhs._ptr);

        if (lhs._font != rhs._font) {
            return false;
        } else if (lhs_value == rhs_value) {
            return true;
        } else {
            return ((lhs_value | rhs_value) & 1) == 0 and *lhs._ptr == *rhs._ptr;
        }
    }

    /** Get information where the glyph is drawn in the atlas.
     */
    [[nodiscard]] glyph_atlas_info& atlas_info() const noexcept;

    /** Get the bounding box and the graphical path of the combined glyphs.
     *
     * The unit of the values are in: em.
     */
    [[nodiscard]] std::pair<graphic_path, aarectangle> get_path_and_bounding_box() const noexcept;

    /** Get the bounding box of the combined glyphs.
     *
     * The unit of the values are in: em.
     */
    [[nodiscard]] aarectangle get_bounding_box() const noexcept;

private:
    static_assert(sizeof(std::size_t) == sizeof(detail::glyph_ids_long *));

    static constexpr std::size_t num_glyphs_shift = 1;
    static constexpr std::size_t num_glyphs_mask = sizeof(std::size_t) == 4 ? 1 : 3;
    static constexpr std::size_t num_graphemes_shift = 4;
    static constexpr std::size_t num_graphemes_mask = 15;

    hi::font const *_font;

    /** A pointer to a list of glyphs.
     *
     * This pointer may also be used for short-string optimization, in that case the bits are:
     *  -     [0] is short string '1' / is pointer '0'.
     *  - [ 3: 1] number of glyphs (zero when empty).
     *  - [ 7: 4] number of graphemes (zero when empty).
     *  - [15: 8] reserved for ratio of grapheme sizes for cursor positioning (set to '0' for even distribution.)
     *  - [31:16] glyph 0
     *  - [47:32] glyph 1
     *  - [63:48] glyph 2
     */
    detail::glyph_ids_long *_ptr;

    constexpr void increment_num_glyphs() noexcept
    {
        hi_axiom(is_short());
        hi_axiom(short_num_glyphs() < num_glyphs_mask);

        _ptr = make_ptr(make_value(_ptr) + (1 << num_glyphs_shift));
    }

    [[nodiscard]] constexpr glyph_id get_glyph(std::size_t index) const noexcept
    {
        hi_axiom(is_short());

        hilet shift = (index + 1) * 16;
        return glyph_id{(make_value(_ptr) >> shift) & 0xffff};
    }

    constexpr void set_glyph(std::size_t i, glyph_id id) noexcept
    {
        hi_axiom(is_short());

        hilet shift = (i + 1) * 16;
        hilet mask = std::size_t{0xffff} << shift;
        _ptr = make_ptr((make_value(_ptr) & ~mask) | (static_cast<std::size_t>(id) << shift));
    }

    [[nodiscard]] constexpr std::size_t short_num_glyphs() const noexcept
    {
        hi_axiom(is_short());
        return (make_value(_ptr) >> num_glyphs_shift) & num_glyphs_mask;
    }

    [[nodiscard]] constexpr bool is_short() const noexcept
    {
        return to_bool(make_value(_ptr) & 1);
    }

    [[nodiscard]] constexpr bool is_long() const noexcept
    {
        return not is_short();
    }

    [[nodiscard]] static constexpr detail::glyph_ids_long *make_ptr(std::size_t value) noexcept
    {
        return std::bit_cast<detail::glyph_ids_long *>(value);
    }

    [[nodiscard]] static constexpr std::size_t make_value(detail::glyph_ids_long *ptr) noexcept
    {
        return std::bit_cast<std::size_t>(ptr);
    }
};

} // namespace hi::inline v1

template<>
struct std::hash<hi::glyph_ids> {
    [[nodiscard]] constexpr std::size_t operator()(hi::glyph_ids const& rhs) const noexcept
    {
        return rhs.hash();
    }
};
