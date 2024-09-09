

// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/with_label_widget.hpp Defines with_label_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "button_delegate.hpp"
#include "label_widget.hpp"
#include "../utility/utility.hpp"
#include "../algorithm/algorithm.hpp"
#include "../l10n/l10n.hpp"
#include "../observer/observer.hpp"
#include "../macros.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>
#include <coroutine>

hi_export_module(hikogui.widgets.with_label_widget);

hi_export namespace hi {
inline namespace v1 {

/** Add labels to a button.
 *
 * @ingroup widgets
 */
template<std::derived_from<widget> ButtonWidget>
class with_label_widget : public widget {
public:
    using super = widget;
    using button_widget_type = ButtonWidget;
    using delegate_type = button_widget_type::delegate_type;

    /** The label to show when the button is in the 'on' state.
     */
    observer<label> on_label = txt("on");

    /** The label to show when the button is in the 'off' state.
     */
    observer<label> off_label = txt("off");

    /** The label to show when the button is in the 'other' state.
     */
    observer<label> other_label = txt("other");

    template<typename... Args>
    [[nodiscard]] static std::shared_ptr<delegate_type> make_default_delegate(Args&&... args)
    {
        return button_widget_type::make_default_delegate(std::forward<Args>(args)...);
    }

    template<std::derived_from<delegate_type> Delegate>
    with_label_widget(std::shared_ptr<Delegate> delegate) noexcept : super()
    {
        _button_widget = std::make_unique<button_widget_type>(std::move(delegate));
        _button_widget->set_parent(this);

        _on_label_widget = std::make_unique<label_widget>(on_label);
        _on_label_widget->set_parent(this);

        _off_label_widget = std::make_unique<label_widget>(off_label);
        _off_label_widget->set_parent(this);

        _other_label_widget = std::make_unique<label_widget>(other_label);
        _other_label_widget->set_parent(this);

        _button_widget_cbt = _button_widget->subscribe([&] {
            this->set_checked(_button_widget->checked());
            this->notifier();
        });

        _button_widget_cbt();

        style.set_name("with-label");
    }

    /** Construct a widget with a label.
     *
     * @param args The arguments to the default button delegate of the embedded
     *             widget followed by arguments to `attributes_type`
     */
    template<typename... Args>
    with_label_widget(Args&&... args) : with_label_widget(make_default_delegate(std::forward<Args>(args)...))
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        // Resolve as if in left-to-right mode, the grid will flip itself.
        auto const resolved_alignment = resolve(style.alignment, true);

        _grid.clear();
        if (resolved_alignment == horizontal_alignment::left) {
            // button label
            _grid.add_cell(0, 0, grid_cell_type::button);
            _grid.add_cell(1, 0, grid_cell_type::label, true);
        } else if (resolved_alignment == horizontal_alignment::right) {
            // label button
            _grid.add_cell(0, 0, grid_cell_type::label, true);
            _grid.add_cell(1, 0, grid_cell_type::button);
        } else if (resolved_alignment == vertical_alignment::top) {
            // button
            // label
            _grid.add_cell(0, 0, grid_cell_type::button);
            _grid.add_cell(0, 1, grid_cell_type::label, true);
        } else if (resolved_alignment == vertical_alignment::bottom) {
            // label
            // button
            _grid.add_cell(0, 0, grid_cell_type::label, true);
            _grid.add_cell(0, 1, grid_cell_type::button);
        } else {
            hi_no_default("alignment is not allowed to be middle-center.");
        }

        for (auto& cell : _grid) {
            if (cell.value == grid_cell_type::button) {
                cell.set_constraints(_button_widget->update_constraints());

            } else if (cell.value == grid_cell_type::label) {
                auto const on_label_constraints = _on_label_widget->update_constraints();
                auto const off_label_constraints = _off_label_widget->update_constraints();
                auto const other_label_constraints = _other_label_widget->update_constraints();
                cell.set_constraints(max(on_label_constraints, off_label_constraints, other_label_constraints));

            } else {
                hi_no_default();
            }
        }

        return _grid.constraints(os_settings::left_to_right(), style.vertical_alignment);
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        super::set_layout(context);

        _grid.set_layout(context.shape);
        for (auto const& cell : _grid) {
            if (cell.value == grid_cell_type::button) {
                _button_widget->set_layout(context.transform(cell.shape, transform_command::level));

            } else if (cell.value == grid_cell_type::label) {
                _on_label_widget->set_layout(context.transform(cell.shape));
                _off_label_widget->set_layout(context.transform(cell.shape));
                _other_label_widget->set_layout(context.transform(cell.shape));

            } else {
                std::unreachable();
            }
        }
    }

    void draw(draw_context const& context) noexcept override
    {
        for (auto const& cell : _grid) {
            if (cell.value == grid_cell_type::button) {
                _button_widget->draw(context);

            } else if (cell.value == grid_cell_type::label) {
                switch (_button_widget->delegate->state(*_button_widget)) {
                case widget_value::on:
                    _on_label_widget->draw(context);
                    break;
                case widget_value::off:
                    _off_label_widget->draw(context);
                    break;
                case widget_value::other:
                    _other_label_widget->draw(context);
                    break;
                default:
                    std::unreachable();
                }

            } else {
                std::unreachable();
            }
        }
    }

    [[nodiscard]] generator<widget_intf&> children(bool include_invisible) noexcept override
    {
        co_yield *_button_widget;
        if (include_invisible or _button_widget->delegate->state(*_button_widget) == widget_value::on) {
            co_yield *_on_label_widget;
        }
        if (include_invisible or _button_widget->delegate->state(*_button_widget) == widget_value::off) {
            co_yield *_off_label_widget;
        }
        if (include_invisible or _button_widget->delegate->state(*_button_widget) == widget_value::other) {
            co_yield *_other_label_widget;
        }
    }

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (enabled() and layout().contains(position)) {
            // Accept the hitbox of the with_label_widget on behalf of the button_widget.
            return {_button_widget->id(), layout().elevation, hitbox_type::button};
        } else {
            return {};
        }
    }
    /// @endprivatesection
protected:
    enum class grid_cell_type { button, label };

    grid_layout<grid_cell_type> _grid;

    std::unique_ptr<button_widget_type> _button_widget;

    std::unique_ptr<label_widget> _on_label_widget;
    std::unique_ptr<label_widget> _off_label_widget;
    std::unique_ptr<label_widget> _other_label_widget;

    callback<void()> _button_widget_cbt;
};

} // namespace v1
} // namespace hi::v1
