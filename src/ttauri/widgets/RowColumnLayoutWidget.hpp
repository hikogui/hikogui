// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ContainerWidget.hpp"
#include "../GUI/Theme.hpp"
#include "../flow_layout.hpp"
#include <memory>

namespace tt {

template<bool IsRow>
class RowColumnLayoutWidget final : public ContainerWidget {
public:
    static constexpr bool is_row = IsRow;

    RowColumnLayoutWidget(Window &window, Widget *parent) noexcept : ContainerWidget(window, parent) {}

    [[nodiscard]] bool updateConstraints() noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (ContainerWidget::updateConstraints()) {
            auto shared_base_line = relative_base_line{VerticalAlignment::Middle, 0.0f, 100};
            auto shared_thickness = finterval{};

            layout.clear();
            ssize_t index = 0;

            for (ttlet &child : children) {
                updateConstraintsForChild(*child, index++, shared_base_line, shared_thickness);
            }

            tt_assume(index == std::ssize(children));

            if constexpr (is_row) {
                _preferred_size = {layout.extent(), shared_thickness};
                _preferred_base_line = shared_base_line;
            } else {
                _preferred_size = {shared_thickness, layout.extent()};
                _preferred_base_line = relative_base_line{};
            }
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        need_layout |= std::exchange(requestLayout, false);
        if (need_layout) {
            layout.update_layout(is_row ? rectangle().width() : rectangle().height());

            ssize_t index = 0;
            for (ttlet &child : children) {
                updateLayoutForChild(*child, index++);
            }

            tt_assume(index == std::ssize(children));
        }
        return ContainerWidget::updateLayout(display_time_point, need_layout);
    }

private:
    flow_layout layout;

    void updateConstraintsForChild(
        Widget const &child,
        ssize_t index,
        relative_base_line &shared_base_line,
        finterval &shared_thickness) noexcept
    {
        ttlet child_lock = std::scoped_lock(child.mutex);

        ttlet length = is_row ? child.preferred_size().width() : child.preferred_size().height();
        ttlet thickness = is_row ? child.preferred_size().height() : child.preferred_size().width();

        ttlet length_resistance = is_row ? child.width_resistance() : child.height_resistance();

        layout.update(index, length, length_resistance, child.margin(), child.preferred_base_line());

        shared_base_line = std::max(shared_base_line, child.preferred_base_line());
        shared_thickness = intersect(shared_thickness, thickness + child.margin() * 2.0f);
    }

    void updateLayoutForChild(Widget &child, ssize_t index) const noexcept
    {
        ttlet child_lock = std::scoped_lock(child.mutex);

        ttlet[child_offset, child_length] = layout.get_offset_and_size(index++);

        ttlet child_rectangle = is_row ?
            aarect{
                rectangle().x() + child_offset,
                rectangle().y() + child.margin(),
                child_length,
                rectangle().height() - child.margin() * 2.0f} :
            aarect{
                rectangle().x() + child.margin(),
                rectangle().y() + child_offset,
                rectangle().width() - child.margin() * 2.0f,
                child_length
            };

        ttlet child_window_rectangle = mat::T2{_window_rectangle} * child_rectangle;

        if constexpr (is_row) {
            child.set_layout_parameters(child_window_rectangle, _window_clipping_rectangle, _window_base_line);
        } else {
            child.set_layout_parameters(child_window_rectangle, _window_clipping_rectangle);
        }
    }
};

using RowLayoutWidget = RowColumnLayoutWidget<true>;
using ColumnLayoutWidget = RowColumnLayoutWidget<false>;

} // namespace tt
