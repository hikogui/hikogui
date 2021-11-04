// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "glyph_id.hpp"
#include "glyph_ids.hpp"
#include "glyph_atlas_info.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../architecture.hpp"
#include <tuple>

namespace tt {
struct graphic_path;
inline namespace v1 {
class font;

class font_glyph_ids {
public:
    constexpr font_glyph_ids(font_glyph_ids const &) noexcept = default;
    constexpr font_glyph_ids(font_glyph_ids &&) noexcept = default;
    constexpr font_glyph_ids &operator=(font_glyph_ids const &) noexcept = default;
    constexpr font_glyph_ids &operator=(font_glyph_ids &&) noexcept = default;

    constexpr font_glyph_ids() noexcept : _font(nullptr), _glyphs() {}
    constexpr font_glyph_ids(tt::font const &font) noexcept : _font(&font), _glyphs() {}

    [[nodiscard]] constexpr font const &font() const noexcept
    {
        tt_axiom(_font);
        return *_font;
    }

    void set_font(tt::font const &font) noexcept
    {
        _font = &font;
    }

    constexpr font_glyph_ids &operator+=(glyph_id rhs) noexcept
    {
        _glyphs += rhs;
        return *this;
    }

    [[nodiscard]] constexpr glyph_id operator[](size_t index) const noexcept
    {
        return _glyphs[index];
    }

    constexpr void clear() noexcept
    {
        _glyphs.clear();
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _glyphs.empty();
    }

    explicit constexpr operator bool () const noexcept
    {
        return not _glyphs.empty();
    }

    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return _glyphs.size();
    }

    [[nodiscard]] glyph_atlas_info &atlas_info() const noexcept;

    [[nodiscard]] constexpr size_t hash() const noexcept
    {
        auto r = std::hash<glyph_ids>{}(_glyphs);
        r ^= std::bit_cast<size_t>(_font);
        return r;
    }

    [[nodiscard]] std::pair<graphic_path, aarectangle> get_path_and_bounding_box() const noexcept;
    [[nodiscard]] aarectangle get_bounding_box() const noexcept;

    [[nodiscard]] constexpr friend bool operator==(font_glyph_ids const &lhs, font_glyph_ids const &rhs) noexcept = default;

private:
    tt::font const *_font;
    glyph_ids _glyphs;    
};

}
} // namespace tt

template<>
struct std::hash<tt::font_glyph_ids> {
    [[nodiscard]] constexpr size_t operator()(tt::font_glyph_ids const &rhs) const noexcept
    {
        return rhs.hash();
    }
};
