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

hi_export_module(hikogui.widgets.menu_button_widget);

hi_export namespace hi { inline namespace v1 {

template<typename Context>
concept menu_button_widget_attribute = label_widget_attribute<Context>;

/** Add menu-button around a small-button.
 *
 * @ingroup widgets
 */
template<std::derived_from<widget> ButtonWidget>
class menu_button_widget : public widget {
public:
    using super = widget;
    using button_widget_type = ButtonWidget;
    using button_attributes_type = button_widget_type::attributes_type;
    using delegate_type = button_widget_type::delegate_type;

    struct attributes_type {
        /** The label to show when the button is in the 'on' state.
         */
        observer<hi::label> label = txt("on");

        /** The label to for the shortcut.
         */
        observer<hi::label> shortcut = hi::label{};

        /** The alignment of the button and on/off/other label.
         */
        observer<alignment> alignment = hi::alignment::middle_left();

        attributes_type(attributes_type const&) noexcept = default;
        attributes_type(attributes_type&&) noexcept = default;
        attributes_type& operator=(attributes_type const&) noexcept = default;
        attributes_type& operator=(attributes_type&&) noexcept = default;

        template<menu_button_widget_attribute... Attributes>
        explicit attributes_type(Attributes&&...attributes) noexcept
        {
            set_attributes<0>(std::forward<Attributes>(attributes)...);
        }

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

            } else {
                hi_static_no_default();
            }
        }
    };

    attributes_type attributes;

    template<typename... Args>
    [[nodiscard]] consteval static size_t num_default_delegate_arguments() noexcept
    {
        return button_widget_type::template num_default_delegate_arguments<Args...>();
    }

    template<size_t N, typename... Args>
    [[nodiscard]] static auto make_default_delegate(Args&&...args)
    {
        return button_widget_type::template make_default_delegate<N, Args...>(std::forward<Args>(args)...);
    }

    hi_call_right_arguments(static, make_attributes, attributes_type);


    menu_button_widget(
        widget_intf const* parent,
        attributes_type attributes,
        std::shared_ptr<delegate_type> delegate) noexcept :
        super(parent), attributes(std::move(attributes))
    {
        _button_widget = std::make_unique<button_widget_type>(
            this, button_attributes_type{this->attributes.alignment, keyboard_focus_group::menu}, std::move(delegate));
        _label_widget =
            std::make_unique<label_widget>(this, this->attributes.label, this->attributes.alignment);
        _shortcut_widget = std::make_unique<label_widget>(
            this, this->attributes.shortcut, this->attributes.alignment);

        // Link the state from the button, so that both this widget and the child widget react in the same way.
        _button_widget->state = state;

        _button_widget_cbt = _button_widget->subscribe([&] {
            this->request_redraw();
            this->notifier();
        });

        _button_widget_cbt();
    }

    /** Construct a widget with a label.
     *
     * @param parent The parent widget that owns this toggle widget.
     * @param args The arguments to the default button delegate of the embedded
     *             widget followed by arguments to `attributes_type`
     */
    template<typename... Args>
    menu_button_widget(widget_intf const* parent, Args&&...args)
        requires(num_default_delegate_arguments<Args...>() != 0)
        :
        menu_button_widget(
            parent,
            make_attributes<num_default_delegate_arguments<Args...>()>(std::forward<Args>(args)...),
            make_default_delegate<num_default_delegate_arguments<Args...>()>(std::forward<Args>(args)...))
    {
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
        constraints.minimum += extent2{theme().template margin<float>() * 2.0f, theme().template margin<float>() * 2.0f};
        constraints.preferred += extent2{theme().template margin<float>() * 2.0f, theme().template margin<float>() * 2.0f};
        constraints.maximum += extent2{theme().template margin<float>() * 2.0f, theme().template margin<float>() * 2.0f};
        constraints.margins = {};
        return constraints;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            auto shape = context.shape;
            shape.rectangle -= theme().template margin<float>();
            _grid.set_layout(shape, theme().baseline_adjustment());
        }

        for (auto const& cell : _grid) {
            if (cell.value == grid_cell_type::button) {
                _button_widget->set_layout(context.transform(cell.shape, transform_command::level));

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
        if (mode() > widget_mode::invisible and overlaps(context, layout())) {
            auto outline_color = focus() ? focus_color() : background_color();
            context.draw_box(
                layout(), layout().rectangle(), background_color(), outline_color, theme().border_width(), border_side::inside);

            for (auto const& cell : _grid) {
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

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (mode() >= widget_mode::partial and layout().contains(position)) {
            // Accept the hitbox of the menu_button_widget on behalf of the button_widget.
            return {_button_widget->id, _layout.elevation, hitbox_type::button};
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

    callback<void()> _button_widget_cbt;
};

}} // namespace hi::v1
