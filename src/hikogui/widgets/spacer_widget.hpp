// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/spacer_widget.hpp Defines spacer_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"

namespace hi { inline namespace v1 {

/** This GUI widget is used as a spacer between other widget for layout purposes.
 * @ingroup widgets
 *
 */
class spacer_widget final : public widget {
public:
    using super = widget;

    spacer_widget(widget *parent) noexcept : super(parent) {}

    [[nodiscard]] generator<widget const &> children(bool include_invisible) const noexcept override
    {
        co_return;
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _layout = {};

        auto r = box_constraints{};
        r.maximum = extent2i::large();
        return r;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        _layout = context;
    }

    void draw(draw_context const& context) noexcept override {}

    [[nodiscard]] hitbox hitbox_test(point2i position) const noexcept override
    {
        return hitbox{};
    }
};

}} // namespace hi::v1
