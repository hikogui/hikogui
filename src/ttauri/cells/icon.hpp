// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ImageCell.hpp"
#include "../URL.hpp"
#include "../PixelMap.hpp"
#include "../R16G16B16A16SFloat.hpp"
#include "../text/FontGlyphIDs.hpp"
#include "../text/ElusiveIcons.hpp"
#include "../text/TTauriIcons.hpp"
#include <variant>

namespace tt {
class DrawContext;

/** An image, in different formats.
 */
class icon {
public:
    icon(URL const &url);
    icon(PixelMap<R16G16B16A16SFloat> &&image) noexcept;
    icon(FontGlyphIDs const &glyph) noexcept;
    icon(ElusiveIcon const &icon) noexcept;
    icon(TTauriIcon const &icon) noexcept;

    icon() noexcept;
    icon(icon const &) noexcept;
    icon(icon &&) noexcept = default;
    icon &operator=(icon const &) noexcept;
    icon &operator=(icon &&) noexcept = default;

    [[nodiscard]] std::unique_ptr<ImageCell> makeCell() const noexcept;

    [[nodiscard]] friend bool operator==(icon const &lhs, icon const &rhs) noexcept
    {
        return lhs.image == rhs.image;
    }

private:
    using image_type = std::variant<std::monostate, FontGlyphIDs, PixelMap<R16G16B16A16SFloat>>;

    image_type image;
};


}
