

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
#include "../algorithm/algorithm.hpp"
#include "../l10n/l10n.hpp"
#include "../observer/observer.hpp"
#include "../coroutine/coroutine.hpp"
#include "../macros.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>
#include <coroutine>

hi_export_module(hikogui.widgets.with_label_widget);

hi_export namespace hi { inline namespace v1 {

template<typename Context>
concept with_label_widget_attribute = label_widget_attribute<Context>;

/** Add labels to a button.
 *
 * @ingroup widgets
 */
template<std::derived_from<widget> ButtonWidget>
class with_label_widget : public widget {
public:
    using super = widget;
    using button_widget_type = ButtonWidget;
    using button_attributes_type = button_widget_type::attributes_type;
    using delegate_type = button_widget_type::delegate_type;

    struct attributes_type {
        /** The label to show when the button is in the 'on' state.
         */
        observer<label> on_label = txt("on");

        /** The label to show when the button is in the 'off' state.
         */
        observer<label> off_label = txt("off");

        /** The label to show when the button is in the 'other' state.
         */
        observer<label> other_label = txt("other");

        /** The alignment of the button and on/off/other label.
         */
        observer<alignment> alignment = hi::alignment::top_left();

        /** The text style to button's label.
         */
        observer<semantic_text_style> text_style = semantic_text_style::label;

        attributes_type(attributes_type const&) noexcept = default;
        attributes_type(attributes_type&&) noexcept = default;
        attributes_type& operator=(attributes_type const&) noexcept = default;
        attributes_type& operator=(attributes_type&&) noexcept = default;

        template<with_label_widget_attribute... Attributes>
        explicit attributes_type(Attributes&&...attributes) noexcept
        {
            set_attributes<0>(std::forward<Attributes>(attributes)...);
        }

        template<size_t I>
        void set_attributes() noexcept
        {
        }

        template<size_t I>
        void set_attributes(with_label_widget_attribute auto&& first, with_label_widget_attribute auto&&...rest) noexcept
        {
            if constexpr (forward_of<decltype(first), observer<hi::label>>) {
                if constexpr (I == 0) {
                    on_label = first;
                    off_label = first;
                    other_label = hi_forward(first);
                } else if constexpr (I == 1) {
                    other_label.reset();
                    off_label.reset();
                    off_label = hi_forward(first);
                } else if constexpr (I == 2) {
                    other_label = hi_forward(first);
                } else {
                    hi_static_no_default();
                }
                set_attributes<I + 1>(hi_forward(rest)...);

            } else if constexpr (forward_of<decltype(first), observer<hi::alignment>>) {
                alignment = hi_forward(first);
                set_attributes<I>(hi_forward(rest)...);

            } else if constexpr (forward_of<decltype(first), observer<hi::semantic_text_style>>) {
                text_style = hi_forward(first);
                set_attributes<I>(hi_forward(rest)...);

            } else {
                hi_static_no_default();
            }
        }
    };

    attributes_type attributes;

    with_label_widget(not_null<widget_intf const *> parent, attributes_type attributes, not_null<std::shared_ptr<delegate_type>> delegate) noexcept :
        super(parent), attributes(std::move(attributes))
    {
        _button_widget =
            std::make_unique<button_widget_type>(this, button_attributes_type{this->attributes.alignment}, std::move(delegate));
        _on_label_widget = std::make_unique<label_widget>(
            this, this->attributes.on_label, this->attributes.alignment, this->attributes.text_style);
        _off_label_widget = std::make_unique<label_widget>(
            this, this->attributes.off_label, this->attributes.alignment, this->attributes.text_style);
        _other_label_widget = std::make_unique<label_widget>(
            this, this->attributes.other_label, this->attributes.alignment, this->attributes.text_style);

        _button_widget_cbt = _button_widget->subscribe([&] {
            auto state_ = state();
            _on_label_widget->mode = state_ == button_state::on ? widget_mode::display : widget_mode::invisible;
            _off_label_widget->mode = state_ == button_state::off ? widget_mode::display : widget_mode::invisible;
            _other_label_widget->mode = state_ == button_state::other ? widget_mode::display : widget_mode::invisible;

            this->request_redraw();
            this->notifier();
        });

        _button_widget_cbt();
    }

    template<with_label_widget_attribute... Attributes>
    with_label_widget(not_null<widget_intf const *> parent, Attributes&&...attributes) noexcept
        requires requires {
            button_widget_type::make_default_delegate();
            attributes_type{std::forward<Attributes>(attributes)...};
        }
        :
        with_label_widget(
            parent,
            attributes_type{std::forward<Attributes>(attributes)...},

            button_widget_type::make_default_delegate())
    {
    }

    template<typename Value, with_label_widget_attribute... Attributes>
    with_label_widget(not_null<widget_intf const *> parent, Value&& value, Attributes&&...attributes) noexcept
        requires requires {
            button_widget_type::make_default_delegate(std::forward<Value>(value));
            attributes_type{std::forward<Attributes>(attributes)...};
        }
        :
        with_label_widget(
            parent,
            attributes_type{std::forward<Attributes>(attributes)...},

            button_widget_type::make_default_delegate(std::forward<Value>(value)))
    {
    }

    template<typename Value, forward_observer<Value> OnValue, with_label_widget_attribute... Attributes>
    with_label_widget(not_null<widget_intf const *> parent, Value&& value, OnValue&& on_value, Attributes&&...attributes) noexcept
        requires requires {
            button_widget_type::make_default_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value));
            attributes_type{std::forward<Attributes>(attributes)...};
        }
        :
        with_label_widget(
            parent,
            attributes_type{std::forward<Attributes>(attributes)...},

            button_widget_type::make_default_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value)))
    {
    }

    template<
        typename Value,
        forward_observer<Value> OnValue,
        forward_observer<Value> OffValue,
        with_label_widget_attribute... Attributes>
    with_label_widget(not_null<widget_intf const *> parent, Value&& value, OnValue&& on_value, OffValue&& off_value, Attributes&&...attributes) noexcept
        requires requires {
            button_widget_type::make_default_delegate(
                std::forward<Value>(value), std::forward<OnValue>(on_value), std::forward<OffValue>(off_value));
            attributes_type{std::forward<Attributes>(attributes)...};
        }
        :
        with_label_widget(
            parent,
            attributes_type{std::forward<Attributes>(attributes)...},
            button_widget_type::make_default_delegate(
                std::forward<Value>(value),
                std::forward<OnValue>(on_value),
                std::forward<OffValue>(off_value)))
    {
    }

    /** Get the current state of the button.
     * @return The state of the button: on / off / other.
     */
    [[nodiscard]] button_state state() const noexcept
    {
        hi_axiom(loop::main().on_thread());
        return _button_widget->state();
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _layout = {};

        // Resolve as if in left-to-right mode, the grid will flip itself.
        hilet resolved_alignment = resolve(*attributes.alignment, true);

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
                hilet on_label_constraints = _on_label_widget->update_constraints();
                hilet off_label_constraints = _off_label_widget->update_constraints();
                hilet other_label_constraints = _other_label_widget->update_constraints();
                cell.set_constraints(max(on_label_constraints, off_label_constraints, other_label_constraints));

            } else {
                hi_no_default();
            }
        }

        return _grid.constraints(os_settings::left_to_right());
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            _grid.set_layout(context.shape, theme().baseline_adjustment());
        }

        for (hilet& cell : _grid) {
            if (cell.value == grid_cell_type::button) {
                _button_widget->set_layout(context.transform(cell.shape, transform_command::level));

            } else if (cell.value == grid_cell_type::label) {
                _on_label_widget->set_layout(context.transform(cell.shape));
                _off_label_widget->set_layout(context.transform(cell.shape));
                _other_label_widget->set_layout(context.transform(cell.shape));

            } else {
                hi_no_default();
            }
        }
    }

    void draw(draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout())) {
            for (hilet& cell : _grid) {
                if (cell.value == grid_cell_type::button) {
                    _button_widget->draw(context);

                } else if (cell.value == grid_cell_type::label) {
                    _on_label_widget->draw(context);
                    _off_label_widget->draw(context);
                    _other_label_widget->draw(context);

                } else {
                    hi_no_default();
                }
            }
        }
    }

    [[nodiscard]] generator<widget_intf&> children(bool include_invisible) noexcept override
    {
        co_yield *_button_widget;
        if (include_invisible or *_on_label_widget->mode > widget_mode::invisible) {
            co_yield *_on_label_widget;
        }
        if (include_invisible or *_off_label_widget->mode > widget_mode::invisible) {
            co_yield *_off_label_widget;
        }
        if (include_invisible or *_other_label_widget->mode > widget_mode::invisible) {
            co_yield *_other_label_widget;
        }
    }

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept final
    {
        hi_axiom(loop::main().on_thread());

        if (*mode >= widget_mode::partial and layout().contains(position)) {
            // Accept the hitbox of the with_label_widget on behalf of the button_widget.
            return {_button_widget->id, _layout.elevation, hitbox_type::button};
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

}} // namespace hi::v1
