// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "icon.hpp"
#include "codec/png.hpp"

namespace tt {

icon::icon() noexcept : _image(std::monostate{}) {}

icon::icon(pixel_map<sfloat_rgba16> &&image) noexcept : _image(std::move(image))
{
    std::get<pixel_map<sfloat_rgba16>>(_image).update_hash();
}

icon::icon(font_glyph_ids const &image) noexcept : _image(image) {}

icon::icon(URL const &url) : icon(png::load(url)) {}

icon::icon(elusive_icon const &icon) noexcept : icon(to_font_glyph_ids(icon)) {}

icon::icon(ttauri_icon const &icon) noexcept : icon(to_font_glyph_ids(icon)) {}

icon::icon(icon const &other) noexcept
{
    tt_axiom(&other != this);
    if (auto font_glyph_id = std::get_if<font_glyph_ids>(&other._image)) {
        _image = *font_glyph_id;

    } else if (auto pixmap = std::get_if<pixel_map<sfloat_rgba16>>(&other._image)) {
        _image = pixmap->copy();

    } else if (std::holds_alternative<std::monostate>(other._image)) {
        _image = std::monostate{};

    } else {
        tt_no_default();
    }
}

icon &icon::operator=(icon const &other) noexcept
{
    // Self-assignment is allowed.
    if (auto font_glyph_id = std::get_if<font_glyph_ids>(&other._image)) {
        _image = *font_glyph_id;

    } else if (auto pixmap = std::get_if<pixel_map<sfloat_rgba16>>(&other._image)) {
        _image = pixmap->copy();

    } else if (std::holds_alternative<std::monostate>(other._image)) {
        _image = std::monostate{};

    } else {
        tt_no_default();
    }
    return *this;
}

} // namespace tt
