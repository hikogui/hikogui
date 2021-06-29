// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "row_column_delegate.hpp"
#include "../GUI/theme.hpp"
#include "../flow_layout.hpp"
#include "../geometry/axis.hpp"
#include <memory>

namespace tt {

template<axis Axis>
class row_column_widget final : public widget {
public:
    using super = widget;
    using delegate_type = row_column_delegate<Axis>;
    static constexpr tt::axis axis = Axis;

    row_column_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate) noexcept :
        super(window, parent), _delegate(std::move(delegate))
    {
        tt_axiom(is_gui_thread());

        if (parent) {
            _semantic_layer = parent->semantic_layer();
        }
        _margin = 0.0f;
    }

    row_column_widget(gui_window &window, widget *parent) noexcept :
        row_column_widget(window, parent, std::weak_ptr<delegate_type>{})
    {
    }

    void init() noexcept override
    {
        super::init();
        if (auto delegate = _delegate.lock()) {
            delegate->init(*this);
        }
    }

    void deinit() noexcept override
    {
        if (auto delegate = _delegate.lock()) {
            delegate->deinit(*this);
        }
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
    {
        tt_axiom(is_gui_thread());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            _layout.clear();
            _layout.reserve(std::ssize(_children));

            ssize_t index = 0;

            auto minimum_thickness = 0.0f;
            auto preferred_thickness = 0.0f;
            auto maximum_thickness = 0.0f;
            for (ttlet &child : _children) {
                update_constraints_for_child(*child, index++, minimum_thickness, preferred_thickness, maximum_thickness);
            }

            tt_axiom(index == std::ssize(_children));

            if constexpr (axis == axis::row) {
                _minimum_size = {_layout.minimum_size(), minimum_thickness};
                _preferred_size = {_layout.preferred_size(), preferred_thickness};
                _maximum_size = {_layout.maximum_size(), maximum_thickness};
            } else {
                _minimum_size = {minimum_thickness, _layout.minimum_size()};
                _preferred_size = {preferred_thickness, _layout.preferred_size()};
                _maximum_size = {maximum_thickness, _layout.maximum_size()};
            }
            tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
    {
        tt_axiom(is_gui_thread());

        need_layout |= _request_relayout.exchange(false);
        if (need_layout) {
            _layout.set_size(axis == axis::row ? rectangle().width() : rectangle().height());

            ssize_t index = 0;
            for (ttlet &child : _children) {
                update_layout_for_child(*child, index++);
            }

            tt_axiom(index == std::ssize(_children));
        }
        super::update_layout(display_time_point, need_layout);
    }

private:
    std::weak_ptr<delegate_type> _delegate;
    flow_layout _layout;

    void update_constraints_for_child(
        widget const &child,
        ssize_t index,
        float &minimum_thickness,
        float &preferred_thickness,
        float &maximum_thickness) noexcept
    {
        tt_axiom(is_gui_thread());

        if (axis == axis::row) {
            ttlet minimum_length = child.minimum_size().width();
            ttlet preferred_length = child.preferred_size().width();
            ttlet maximum_length = child.maximum_size().width();
            _layout.update(index, minimum_length, preferred_length, maximum_length, child.margin());

            minimum_thickness = std::max(minimum_thickness, child.minimum_size().height() + child.margin() * 2.0f);
            preferred_thickness = std::max(preferred_thickness, child.preferred_size().height() + child.margin() * 2.0f);
            maximum_thickness = std::max(maximum_thickness, child.maximum_size().height() + child.margin() * 2.0f);

        } else {
            ttlet minimum_length = child.minimum_size().height();
            ttlet preferred_length = child.preferred_size().height();
            ttlet maximum_length = child.maximum_size().height();
            _layout.update(index, minimum_length, preferred_length, maximum_length, child.margin());

            minimum_thickness = std::max(minimum_thickness, child.minimum_size().width() + child.margin() * 2.0f);
            preferred_thickness = std::max(preferred_thickness, child.preferred_size().width() + child.margin() * 2.0f);
            maximum_thickness = std::max(maximum_thickness, child.maximum_size().width() + child.margin() * 2.0f);
        }
    }

    void update_layout_for_child(widget &child, ssize_t index) const noexcept
    {
        tt_axiom(is_gui_thread());

        ttlet[child_offset, child_length] = _layout.get_offset_and_size(index++);

        ttlet child_rectangle = axis == axis::row ?
            aarectangle{
                rectangle().left() + child_offset,
                rectangle().bottom() + child.margin(),
                child_length,
                rectangle().height() - child.margin() * 2.0f} :
            aarectangle{
                rectangle().left() + child.margin(),
                rectangle().top() - child_offset - child_length,
                rectangle().width() - child.margin() * 2.0f,
                child_length};

        child.set_layout_parameters_from_parent(child_rectangle);
    }
};

using row_widget = row_column_widget<axis::row>;
using column_widget = row_column_widget<axis::column>;

} // namespace tt
