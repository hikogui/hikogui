// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "icon.hpp"
#include "codec/png.hpp"

namespace tt::inline v1 {

icon::icon(pixel_map<sfloat_rgba16> &&image) noexcept : _image(std::move(image)) {}

icon::icon(glyph_ids const &image) noexcept : _image(image) {}

icon::icon(URL const &url) : icon(png::load(url)) {}

icon::icon(elusive_icon const &icon) noexcept : _image(icon) {}

icon::icon(ttauri_icon const &icon) noexcept : _image(icon) {}

} // namespace tt::inline v1
