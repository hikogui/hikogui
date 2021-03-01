// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_container_widget.hpp"
#include "../GUI/theme.hpp"
#include "../flow_layout.hpp"
#include "../alignment.hpp"
#include <memory>

namespace tt {

template<arrangement Arrangement>
class row_column_layout_widget final : public abstract_container_widget {
public:
    using super = abstract_container_widget;
    static constexpr auto arrangement = Arrangement;

    row_column_layout_widget(gui_window &window, std::shared_ptr<abstract_container_widget> parent) noexcept :
        super(window, parent)
    {
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            auto shared_base_line = relative_base_line{vertical_alignment::middle, 0.0f, 100};
            auto shared_thickness = 0.0f;

            _layout.clear();
            _layout.reserve(std::ssize(_children));

            ssize_t index = 0;
            for (ttlet &child : _children) {
                update_constraints_for_child(*child, index++, shared_base_line, shared_thickness);
            }

            tt_axiom(index == std::ssize(_children));

            if constexpr (arrangement == arrangement::row) {
                _preferred_size = {_layout.minimum_size(), shared_thickness};
            } else {
                _preferred_size = {shared_thickness, _layout.minimum_size()};
            }
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            _layout.set_size(arrangement == arrangement::row ? rectangle().width() : rectangle().height());

            ssize_t index = 0;
            for (ttlet &child : _children) {
                update_layout_for_child(*child, index++);
            }

            tt_axiom(index == std::ssize(_children));
        }
        abstract_container_widget::update_layout(display_time_point, need_layout);
    }

private:
    flow_layout _layout;

    void update_constraints_for_child(
        widget const &child,
        ssize_t index,
        relative_base_line &shared_base_line,
        float &shared_thickness) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        ttlet length = arrangement == arrangement::row ? child.preferred_size().minimum().width() :
                                                         child.preferred_size().minimum().height();
        ttlet thickness = arrangement == arrangement::row ? child.preferred_size().minimum().height() :
                                                            child.preferred_size().minimum().width();

        ttlet length_resistance = arrangement == arrangement::row ? child.width_resistance() : child.height_resistance();

        _layout.update(index, length, length_resistance, child.margin());

        shared_thickness = std::max(shared_thickness, thickness + child.margin() * 2.0f);
    }

    void update_layout_for_child(widget &child, ssize_t index) const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        ttlet[child_offset, child_length] = _layout.get_offset_and_size(index++);

        ttlet child_rectangle = arrangement == arrangement::row ?
            aarect{
                rectangle().left() + child_offset,
                rectangle().bottom() + child.margin(),
                child_length,
                rectangle().height() - child.margin() * 2.0f} :
            aarect{
                rectangle().left() + child.margin(),
                rectangle().top() - child_offset - child_length,
                rectangle().width() - child.margin() * 2.0f,
                child_length
            };


        child.set_layout_parameters_from_parent(child_rectangle);
    }
};

using row_layout_widget = row_column_layout_widget<arrangement::row>;
using column_layout_widget = row_column_layout_widget<arrangement::column>;

} // namespace tt
