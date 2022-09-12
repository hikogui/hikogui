// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "URL.hpp"
#include "pixel_map.hpp"
#include "rapid/sfloat_rgba16.hpp"
#include "text/glyph_ids.hpp"
#include "text/elusive_icon.hpp"
#include "text/hikogui_icon.hpp"
#include <variant>

namespace hi::inline v1 {

/** An image, in different formats.
 */
class icon {
public:
    icon(URL const &url);
    icon(pixel_map<sfloat_rgba16> &&image) noexcept;
    icon(glyph_ids const &glyph) noexcept;
    icon(elusive_icon const &icon) noexcept;
    icon(hikogui_icon const &icon) noexcept;

    constexpr icon() noexcept : _image(std::monostate{}) {}

    icon(icon const &) noexcept = default;
    icon(icon &&) noexcept = default;
    icon &operator=(icon const &) noexcept = default;
    icon &operator=(icon &&) noexcept = default;

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return std::holds_alternative<std::monostate>(_image);
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] friend bool operator==(icon const &lhs, icon const &rhs) noexcept = default;

    template<typename T>
    [[nodiscard]] friend bool holds_alternative(hi::icon const &icon) noexcept
    {
        return std::holds_alternative<T>(icon._image);
    }

    template<typename T>
    [[nodiscard]] friend T const &get(hi::icon const &icon) noexcept
    {
        return std::get<T>(icon._image);
    }

    template<typename T>
    [[nodiscard]] friend T &get(hi::icon &icon) noexcept
    {
        return std::get<T>(icon._image);
    }

    template<typename T>
    [[nodiscard]] friend std::add_pointer_t<T const> get_if(hi::icon const *icon) noexcept
    {
        return std::get_if<T>(&icon->_image);
    }

    template<typename T>
    [[nodiscard]] friend std::add_pointer_t<T> get_if(hi::icon *icon) noexcept
    {
        return std::get_if<T>(&icon->_image);
    }

private:
    using image_type = std::variant<std::monostate, elusive_icon, hikogui_icon, glyph_ids, pixel_map<sfloat_rgba16>>;

    image_type _image;

    friend class stencil;
};

} // namespace hi::inline v1
