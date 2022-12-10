// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/box_constraints.hpp Defines box_constraints.
 * @ingroup widget_utilities
 */

#include "../GFX/gfx_surface.hpp"
#include "../GUI/theme.hpp"
#include "../text/font_book.hpp"

namespace hi { inline namespace v1 {

class get_constraints_context {
public:
    hi::font_book *font_book = nullptr;
    hi::theme const *theme = nullptr;
    gfx_surface *surface = nullptr;
    unicode_bidi_class writing_direction = unicode_bidi_class::L;

    constexpr get_constraints_context() = default;
    constexpr get_constraints_context(get_constraints_context const&) noexcept = default;
    constexpr get_constraints_context(get_constraints_context&&) noexcept = default;
    constexpr get_constraints_context& operator=(get_constraints_context const&) noexcept = default;
    constexpr get_constraints_context& operator=(get_constraints_context&&) noexcept = default;

    constexpr get_constraints_context(
        hi::font_book *font_book,
        hi::theme const *theme,
        unicode_bidi_class writing_direction,
        gfx_surface *surface = nullptr) noexcept :
        font_book(font_book), theme(theme), surface(surface), writing_direction(writing_direction)
    {
    }

    constexpr get_constraints_context(
        hi::font_book& font_book,
        hi::theme const& theme,
        unicode_bidi_class writing_direction,
        gfx_surface& surface) noexcept :
        font_book(&font_book), theme(&theme), surface(&surface), writing_direction(writing_direction)
    {
    }

    [[nodiscard]] constexpr bool left_to_right() const noexcept
    {
        return writing_direction == unicode_bidi_class::L;
    }
};

}} // namespace hi::v1
