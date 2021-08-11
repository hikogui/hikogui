// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "URL.hpp"
#include "pixel_map.hpp"
#include "rapid/sfloat_rgba16.hpp"
#include "text/font_glyph_ids.hpp"
#include "text/elusive_icon.hpp"
#include "text/ttauri_icon.hpp"
#include <variant>

namespace tt {

/** An image, in different formats.
 */
class icon {
public:
    icon(URL const &url);
    icon(pixel_map<sfloat_rgba16> &&image) noexcept;
    icon(font_glyph_ids const &glyph) noexcept;
    icon(elusive_icon const &icon) noexcept;
    icon(ttauri_icon const &icon) noexcept;

    constexpr icon() noexcept : _image(std::monostate{}) {}

    icon(icon const &) noexcept = default;
    icon(icon &&) noexcept = default;
    icon &operator=(icon const &) noexcept = default;
    icon &operator=(icon &&) noexcept = default;

    [[nodiscard]] explicit operator bool () const noexcept
    {
        return !std::holds_alternative<std::monostate>(_image);
    }

    [[nodiscard]] friend bool operator==(icon const &lhs, icon const &rhs) noexcept
    {
        return lhs._image == rhs._image;
    }

    template<typename T>
    [[nodiscard]] friend bool holds_alternative(tt::icon const &icon) noexcept
    {
        return std::holds_alternative<T>(icon._image);
    }

    template<typename T>
    [[nodiscard]] friend T const &get(tt::icon const &icon) noexcept
    {
        return std::get<T>(icon._image);
    }

    template<typename T>
    [[nodiscard]] friend T &get(tt::icon &icon) noexcept
    {
        return std::get<T>(_image);
    }

private:
    using image_type = std::variant<std::monostate, elusive_icon, ttauri_icon, font_glyph_ids, pixel_map<sfloat_rgba16>>;

    image_type _image;

    friend class stencil;
};


}
