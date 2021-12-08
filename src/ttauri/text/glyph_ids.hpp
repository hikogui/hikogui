// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "glyph_id.hpp"
#include "../required.hpp"
#include <bit>
#include <memory>
#include <cstddef>
#include <array>
#include <cstdint>
#include <functional>

namespace tt::inline v1 {
class glyph_ids_long {
public:
    constexpr glyph_ids_long(glyph_ids_long const &) noexcept = default;
    constexpr glyph_ids_long(glyph_ids_long &&) noexcept = default;
    constexpr glyph_ids_long &operator=(glyph_ids_long const &) noexcept = default;
    constexpr glyph_ids_long &operator=(glyph_ids_long &&) noexcept = default;

    /** Construct a list of glyphs starting with a packed set of glyphs.
     *
     * @param value The value contains a set of glyphs packed into a std::size_t,
     *              the first glyph is in the least significant bits.
     */
    constexpr glyph_ids_long(std::size_t value) noexcept : _glyphs(), _size(0)
    {
        *this += glyph_id{value & 0xffff};

        if constexpr (sizeof(std::size_t) == 8) {
            *this += glyph_id{(value >> 16) & 0xffff};
            *this += glyph_id{(value >> 32) & 0xffff};
        }
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return _size;
    }

    [[nodiscard]] constexpr std::size_t hash() const noexcept
    {
        std::size_t r = 0;

        if constexpr (sizeof(std::size_t) == 8) {
            r |= std::size_t{get<3>(_glyphs)};
            r <<= 16;
            r |= std::size_t{get<2>(_glyphs)};
            r <<= 16;
        }
        r |= std::size_t{get<1>(_glyphs)};
        r <<= 16;
        r |= std::size_t{get<0>(_glyphs)};

        return r ^ std::size_t { _size };
    }

    constexpr glyph_ids_long &operator+=(glyph_id id) noexcept
    {
        if (_size < _glyphs.size()) {
            _glyphs[_size++] = id;
        }
        return *this;
    }

    [[nodiscard]] constexpr glyph_id const &operator[](std::size_t i) const noexcept
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

class glyph_ids {
public:
    static constexpr std::size_t sgo_size_max = sizeof(void *) == 8 ? 3 : 1;
    static constexpr std::size_t sgo_size_mask = sizeof(void *) == 8 ? 7 : 3;
    static constexpr glyph_ids_long *sgo_null = reinterpret_cast<glyph_ids_long *>(1);

    constexpr ~glyph_ids()
    {
        if ((sgo_value() & sgo_size_mask) == 0) {
            delete _ptr;
        }
    }

    constexpr glyph_ids(glyph_ids const &other) noexcept : _ptr(other._ptr)
    {
        if ((sgo_value() & sgo_size_mask) == 0) {
            _ptr = new glyph_ids_long(*(other._ptr));
        }
    }

    constexpr glyph_ids &operator=(glyph_ids const &other) noexcept
    {
        tt_return_on_self_assignment(other);

        if ((sgo_value() & sgo_size_mask) == 0) {
            delete _ptr;
        }
        _ptr = other._ptr;
        if ((sgo_value() & sgo_size_mask) == 0) {
            _ptr = new glyph_ids_long(*(other._ptr));
        }
        return *this;
    }

    constexpr glyph_ids(glyph_ids &&other) noexcept : _ptr(std::exchange(other._ptr, sgo_null)) {}

    constexpr glyph_ids &operator=(glyph_ids &&other) noexcept
    {
        std::swap(_ptr, other._ptr);
        return *this;
    }

    constexpr glyph_ids() noexcept : _ptr(sgo_null) {}

    constexpr void clear() noexcept
    {
        if ((sgo_value() & sgo_size_mask) == 0) {
            delete _ptr;
        }
        _ptr = sgo_null;
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _ptr == sgo_null;
    }

    [[nodiscard]] constexpr bool is_single() const noexcept
    {
        return (sgo_value() & sgo_size_mask) == 2;
    }

    [[nodiscard]] constexpr glyph_id get_single() const noexcept
    {
        tt_axiom(is_single());
        return glyph_id{(sgo_value() >> 16) & 0xffff};
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        ttlet size = sgo_value() & sgo_size_mask;
        if (size == 0) {
            return _ptr->size();
        } else {
            return size - 1;
        }
    }

    [[nodiscard]] constexpr std::size_t hash() const noexcept
    {
        ttlet value = sgo_value();
        if ((value & sgo_size_mask) == 0) {
            return _ptr->hash();
        } else {
            return value;
        }
    }

    constexpr glyph_ids &operator+=(glyph_id id) noexcept
    {
        ttlet value = sgo_value();
        ttlet size_code = value & sgo_size_mask;

        if (size_code == 0) {
            *_ptr += id;

        } else if (size_code <= sgo_size_max) {
            set_sgo_value((size_code + 1) | (std::size_t{id} << (size_code * 16)));

        } else {
            _ptr = new glyph_ids_long(value >> 16);
            *_ptr += id;
        }
        return *this;
    }

    [[nodiscard]] constexpr glyph_id operator[](std::size_t i) const noexcept
    {
        tt_axiom(i < size());

        ttlet value = sgo_value();
        if ((value & sgo_size_mask) == 0) {
            return (*_ptr)[i];
        } else {
            return glyph_id{value >> ((i + 1) * 16) & 0xffff};
        }
    }

    [[nodiscard]] constexpr friend bool operator==(glyph_ids const &lhs, glyph_ids const &rhs) noexcept
    {
        ttlet lhs_value = lhs.sgo_value();
        ttlet rhs_value = rhs.sgo_value();
        ttlet lhs_sgo_size = lhs_value & sgo_size_mask;
        ttlet rhs_sgo_size = rhs_value & sgo_size_mask;

        if (lhs_sgo_size != rhs_sgo_size) {
            return false;
        } else if (lhs_sgo_size == 0) {
            return *lhs._ptr == *rhs._ptr;
        } else {
            return lhs_value == rhs_value;
        }
    }

private:
    glyph_ids_long *_ptr;

    constexpr void set_sgo_value(std::size_t value) noexcept
    {
        _ptr = std::bit_cast<glyph_ids_long *>(value);
    }

    [[nodiscard]] constexpr std::size_t sgo_value() const noexcept
    {
        return std::bit_cast<std::size_t>(_ptr);
    }
};

} // namespace tt::inline v1

template<>
struct std::hash<tt::glyph_ids> {
    [[nodiscard]] constexpr std::size_t operator()(tt::glyph_ids const &rhs) const noexcept
    {
        return rhs.hash();
    }
};
