// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/menu_button_widget.hpp Defines menu_button_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "button_delegate.hpp"
#include "label_widget.hpp"
#include "../algorithm/module.hpp"
#include "../l10n/l10n.hpp"
#include "../observer/module.hpp"
#include "../macros.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace hi { inline namespace v1 {

template<typename Context>
concept menu_button_widget_attribute = label_widget_attribute<Context>;

/** Add labels to a button.
 *
 * @ingroup widgets
 */
template<std::derived_from<widget> ButtonWidget>
class menu_button_widget : public widget {
public:
    using super = widget;
    using button_widget_type = ButtonWidget;
    using delegate_type = button_widget_type::delegate_type;

    /** The label to show when the button is in the 'on' state.
     */
    observer<hi::label> label = txt("on");

    /** The label to for the shortcut.
     */
    observer<hi::label> shortcut = hi::label{};

    /** The alignment of the button and on/off/other label.
     */
    observer<alignment> alignment;

    /** The text style to button's label.
     */
    observer<semantic_text_style> text_style = semantic_text_style::label;

    notifier<void()> activated;

    template<menu_button_widget_attribute... Attributes>
    menu_button_widget(widget *parent, std::shared_ptr<delegate_type> delegate, Attributes&&...attributes) noexcept :
        super(parent)
    {
        hi_assert_not_null(delegate);

        alignment = alignment::middle_left();
        set_attributes<0>(std::forward<Attributes>(attributes)...);

        _button_widget = std::make_unique<button_widget_type>(this, std::move(delegate), alignment, keyboard_focus_group::menu);
        _label_widget = std::make_unique<label_widget>(this, label, alignment, text_style);
        _shortcut_widget = std::make_unique<label_widget>(this, shortcut, alignment, text_style);

        // Link the focus from the button, so that we can draw a focus ring
        // around the whole menu item.
        _button_widget->focus = focus;
        _button_widget->hover = hover;

        _button_widget_activated_cbt = _button_widget->activated.subscribe([&] {
            this->request_redraw();
            this->activated();
        });

        _button_widget_activated_cbt();
    }

    template<different_from<std::shared_ptr<delegate_type>> Value, menu_button_widget_attribute... Attributes>
    menu_button_widget(widget *parent, Attributes&&...attributes) noexcept
        requires requires { button_widget_type::make_default_delegate(); }
        :
        menu_button_widget(
            parent,
            button_widget_type::make_default_delegate(),
            std::forward<Attributes>(attributes)...)
    {
        _button_widget->alignment = alignment;
    }

    template<different_from<std::shared_ptr<delegate_type>> Value, menu_button_widget_attribute... Attributes>
    menu_button_widget(widget *parent, Value&& value, Attributes&&...attributes) noexcept
        requires requires { button_widget_type::make_default_delegate(std::forward<Value>(value)); }
        :
        menu_button_widget(
            parent,
            button_widget_type::make_default_delegate(std::forward<Value>(value)),
            std::forward<Attributes>(attributes)...)
    {
        _button_widget->alignment = alignment;
    }

    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_observer<Value> OnValue,
        menu_button_widget_attribute... Attributes>
    menu_button_widget(widget *parent, Value&& value, OnValue&& on_value, Attributes&&...attributes) noexcept
        requires requires {
            button_widget_type::make_default_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value));
        }
        :
        menu_button_widget(
            parent,
            button_widget_type::make_default_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value)),
            std::forward<Attributes>(attributes)...)
    {
        _button_widget->alignment = alignment;
    }

    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_observer<Value> OnValue,
        forward_observer<Value> OffValue,
        menu_button_widget_attribute... Attributes>
    menu_button_widget(widget *parent, Value&& value, OnValue&& on_value, OffValue&& off_value, Attributes&&...attributes) noexcept
        requires requires {
            button_widget_type::make_default_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value), std::forward<OffValue>(off_value));
        }
        :
        menu_button_widget(
            parent,
            button_widget_type::make_default_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value), std::forward<OffValue>(off_value)),
            std::forward<Attributes>(attributes)...)
    {
        _button_widget->alignment = alignment;
    }

    /** Get the current state of the button.
     * @return The state of the button: on / off / other.
     */
    [[nodiscard]] button_state state() const noexcept
    {
        hi_axiom(loop::main().on_thread());
        return _button_widget->state();
    }

    /** Get the id of the widget that takes keyboard focus
     */
    [[nodiscard]] widget_id focus_id() const noexcept
    {
        return _button_widget->id;
    }

    template<forward_of<void()> Func>
    [[nodiscard]] callback<void()> subscribe(Func &&func, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return activated.subscribe(std::forward<Func>(func), flags);
    }

    [[nodiscard]] auto operator co_await() noexcept
    {
        return activated.operator co_await();
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _layout = {};

        _grid.clear();
        _grid.add_cell(0, 0, grid_cell_type::button);
        _grid.add_cell(1, 0, grid_cell_type::label, true);
        _grid.add_cell(2, 0, grid_cell_type::shortcut);

        for (auto& cell : _grid) {
            if (cell.value == grid_cell_type::button) {
                auto constraints = _button_widget->update_constraints();
                inplace_max(constraints.minimum.width(), theme().size() * 2.0f);
                inplace_max(constraints.preferred.width(), theme().size() * 2.0f);
                inplace_max(constraints.maximum.width(), theme().size() * 2.0f);
                cell.set_constraints(constraints);

            } else if (cell.value == grid_cell_type::label) {
                cell.set_constraints(_label_widget->update_constraints());

            } else if (cell.value == grid_cell_type::shortcut) {
                auto constraints = _shortcut_widget->update_constraints();
                inplace_max(constraints.minimum.width(), theme().size() * 3.0f);
                inplace_max(constraints.preferred.width(), theme().size() * 3.0f);
                inplace_max(constraints.maximum.width(), theme().size() * 3.0f);
                cell.set_constraints(constraints);

            } else {
                hi_no_default();
            }
        }

        auto constraints = _grid.constraints(os_settings::left_to_right());
        constraints.minimum += extent2{theme().margin<float>() * 2.0f, theme().margin<float>() * 2.0f};
        constraints.preferred += extent2{theme().margin<float>() * 2.0f, theme().margin<float>() * 2.0f};
        constraints.maximum += extent2{theme().margin<float>() * 2.0f, theme().margin<float>() * 2.0f};
        constraints.margins = {};
        return constraints;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            auto shape = context.shape;
            shape.rectangle -= theme().margin<float>();
            _grid.set_layout(shape, theme().baseline_adjustment());
        }

        for (hilet& cell : _grid) {
            if (cell.value == grid_cell_type::button) {
                _button_widget->set_layout(context.transform(cell.shape, 0.0f));

            } else if (cell.value == grid_cell_type::label) {
                _label_widget->set_layout(context.transform(cell.shape));

            } else if (cell.value == grid_cell_type::shortcut) {
                _shortcut_widget->set_layout(context.transform(cell.shape));

            } else {
                hi_no_default();
            }
        }
    }

    void draw(draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout())) {
            auto outline_color = *focus ? focus_color() : background_color();
            context.draw_box(
                layout(), layout().rectangle(), background_color(), outline_color, theme().border_width(), border_side::inside);

            for (hilet& cell : _grid) {
                if (cell.value == grid_cell_type::button) {
                    _button_widget->draw(context);

                } else if (cell.value == grid_cell_type::label) {
                    _label_widget->draw(context);

                } else if (cell.value == grid_cell_type::shortcut) {
                    _shortcut_widget->draw(context);

                } else {
                    hi_no_default();
                }
            }
        }
    }

    [[nodiscard]] generator<widget_intf&> children(bool include_invisible) noexcept override
    {
        co_yield *_button_widget;
        co_yield *_label_widget;
        co_yield *_shortcut_widget;
    }

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept final
    {
        hi_axiom(loop::main().on_thread());

        if (*mode >= widget_mode::partial and layout().contains(position)) {
            // Accept the hitbox of the menu_button_widget on behalf of the button_widget.
            return {focus_id(), _layout.elevation, hitbox_type::button};
        } else {
            return {};
        }
    }
    /// @endprivatesection
protected:
    enum class grid_cell_type { button, label, shortcut };

    grid_layout<grid_cell_type> _grid;

    std::unique_ptr<button_widget_type> _button_widget;

    std::unique_ptr<label_widget> _label_widget;
    std::unique_ptr<label_widget> _shortcut_widget;

    callback<void()> _button_widget_activated_cbt;

    template<size_t I>
    void set_attributes() noexcept
    {
    }

    template<size_t I, menu_button_widget_attribute First, menu_button_widget_attribute... Rest>
    void set_attributes(First&& first, Rest&&...rest) noexcept
    {
        if constexpr (forward_of<First, observer<hi::label>>) {
            if constexpr (I == 0) {
                label = std::forward<First>(first);
            } else if constexpr (I == 1) {
                shortcut = std::forward<First>(first);
            } else {
                hi_static_no_default();
            }
            set_attributes<I + 1>(std::forward<Rest>(rest)...);

        } else if constexpr (forward_of<First, observer<hi::alignment>>) {
            alignment = std::forward<First>(first);
            set_attributes<I>(std::forward<Rest>(rest)...);

        } else if constexpr (forward_of<First, observer<hi::semantic_text_style>>) {
            text_style = std::forward<First>(first);
            set_attributes<I>(std::forward<Rest>(rest)...);

        } else {
            hi_static_no_default();
        }
    }
};

}} // namespace hi::v1
