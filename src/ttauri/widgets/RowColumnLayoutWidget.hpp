// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "abstract_container_widget.hpp"
#include "../GUI/Theme.hpp"
#include "../flow_layout.hpp"
#include <memory>

namespace tt {

template<bool IsRow>
class RowColumnLayoutWidget final : public abstract_container_widget {
public:
    static constexpr bool is_row = IsRow;

    RowColumnLayoutWidget(Window &window, std::shared_ptr<widget> parent) noexcept : abstract_container_widget(window, parent) {}

    [[nodiscard]] bool update_constraints() noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (abstract_container_widget::update_constraints()) {
            auto shared_base_line = relative_base_line{VerticalAlignment::Middle, 0.0f, 100};
            auto shared_thickness = finterval{};

            layout.clear();
            layout.reserve(std::ssize(_children));

            ssize_t index = 0;
            for (ttlet &child : _children) {
                updateConstraintsForChild(*child, index++, shared_base_line, shared_thickness);
            }

            tt_assume(index == std::ssize(_children));

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

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            layout.update_layout(is_row ? rectangle().width() : rectangle().height());

            ssize_t index = 0;
            for (ttlet &child : _children) {
                updateLayoutForChild(*child, index++);
            }

            tt_assume(index == std::ssize(_children));
        }
        return abstract_container_widget::update_layout(display_time_point, need_layout);
    }

private:
    flow_layout layout;

    void updateConstraintsForChild(
        widget const &child,
        ssize_t index,
        relative_base_line &shared_base_line,
        finterval &shared_thickness) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        ttlet length = is_row ? child.preferred_size().width() : child.preferred_size().height();
        ttlet thickness = is_row ? child.preferred_size().height() : child.preferred_size().width();

        ttlet length_resistance = is_row ? child.width_resistance() : child.height_resistance();

        layout.update(index, length, length_resistance, child.margin(), child.preferred_base_line());

        shared_base_line = std::max(shared_base_line, child.preferred_base_line());
        shared_thickness = intersect(shared_thickness, thickness + child.margin() * 2.0f);
    }

    void updateLayoutForChild(widget &child, ssize_t index) const noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

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
