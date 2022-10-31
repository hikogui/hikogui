// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/widget_constraints.hpp Defines widget_constraints.
 * @ingroup widget_utilities
 */

#include "../GFX/gfx_surface.hpp"
#include "../GUI/theme.hpp"
#include "../text/font_book.hpp"

namespace hi { inline namespace v1 {

class set_constraints_context {
public:
    hi::font_book *font_book = nullptr;
    hi::theme const *theme = nullptr;
    gfx_surface *surface = nullptr;

    constexpr set_constraints_context() = default;
    constexpr set_constraints_context(set_constraints_context const&) noexcept = default;
    constexpr set_constraints_context(set_constraints_context&&) noexcept = default;
    constexpr set_constraints_context& operator=(set_constraints_context const&) noexcept = default;
    constexpr set_constraints_context& operator=(set_constraints_context&&) noexcept = default;

    constexpr set_constraints_context(hi::font_book *font_book, hi::theme const *theme, gfx_surface *surface = nullptr) noexcept :
        font_book(font_book), theme(theme), surface(surface)
    {
    }

    constexpr set_constraints_context(hi::font_book& font_book, hi::theme const& theme, gfx_surface& surface) noexcept :
        font_book(&font_book), theme(&theme), surface(&surface)
    {
    }
};

}} // namespace hi::v1
