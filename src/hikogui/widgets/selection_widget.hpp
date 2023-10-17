// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/selection_widget.hpp Defines selection_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "label_widget.hpp"
#include "overlay_widget.hpp"
#include "scroll_widget.hpp"
#include "grid_widget.hpp"
#include "radio_button_widget.hpp"
#include "selection_delegate.hpp"
#include "../observer/module.hpp"
#include "../macros.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace hi { inline namespace v1 {

template<typename Context>
concept selection_widget_attribute = label_widget_attribute<Context>;

/** A graphical control element that allows the user to choose only one of a
 * predefined set of mutually exclusive options.
 *
 * @image html selection_widget.gif
 *
 * The following example creates a selection widget with three options.
 * which will monitor and modify `value` to display the options from
 * the `option_list`. At application startup, value is zero and none
 * of the options is selected:
 *
 * @snippet widgets/selection_example_impl.cpp Create selection
 *
 * @ingroup widgets
 */
class selection_widget final : public widget {
public:
    using super = widget;
    using delegate_type = selection_delegate;

    struct attributes_type {
        /** The label to show when nothing is selected.
         */
        observer<label> off_label;

        observer<alignment> alignment = hi::alignment::middle_left();

        /** The text style to display the label's text in and color of the label's (non-color) icon.
         */
        observer<semantic_text_style> text_style = semantic_text_style::label;

        attributes_type(attributes_type const&) noexcept = default;
        attributes_type(attributes_type&&) noexcept = default;
        attributes_type& operator=(attributes_type const&) noexcept = default;
        attributes_type& operator=(attributes_type&&) noexcept = default;

        template<selection_widget_attribute... Attributes>
        explicit attributes_type(Attributes&&...attributes) noexcept
        {
            set_attributes(std::forward<Attributes>(attributes)...);
        }

        void set_attributes() noexcept {}
        void set_attributes(selection_widget_attribute auto&& first, selection_widget_attribute auto&&...rest) noexcept
        {
            if constexpr (forward_of<decltype(first), observer<hi::label>>) {
                off_label = hi_forward(first);
            } else if constexpr (forward_of<decltype(first), observer<hi::alignment>>) {
                alignment = hi_forward(first);
            } else if constexpr (forward_of<decltype(first), observer<hi::semantic_text_style>>) {
                text_style = hi_forward(first);
            } else {
                hi_static_no_default();
            }

            set_attributes(hi_forward(rest)...);
        }
    };

    attributes_type attributes;

    not_null<std::shared_ptr<delegate_type>> delegate;

    template<typename... Args>
    [[nodiscard]] static not_null<std::shared_ptr<delegate_type>> make_default_delegate(Args &&...args)
        requires requires { make_shared_ctad_not_null<default_selection_delegate>(std::forward<Args>(args)...); }
    {
        return make_shared_ctad_not_null<default_selection_delegate>(std::forward<Args>(args)...);
    }

    ~selection_widget()
    {
        delegate->deinit(*this);
    }

    /** Construct a selection widget with a delegate.
     *
     * @param parent The owner of the selection widget.
     * @param delegate The delegate which will control the selection widget.
     */
    selection_widget(widget *parent, attributes_type attributes, not_null<std::shared_ptr<delegate_type>> delegate) noexcept :
        super(parent), attributes(std::move(attributes)), delegate(std::move(delegate))
    {
        _current_label_widget = std::make_unique<label_widget>(this, this->attributes.alignment, this->attributes.text_style);
        _current_label_widget->mode = widget_mode::invisible;
        _off_label_widget = std::make_unique<label_widget>(this, this->attributes.off_label, this->attributes.alignment, semantic_text_style::placeholder);

        _overlay_widget = std::make_unique<overlay_widget>(this);
        _overlay_widget->mode = widget_mode::invisible;
        _scroll_widget = &_overlay_widget->emplace<vertical_scroll_widget>();
        _grid_widget = &_scroll_widget->emplace<grid_widget>();

        _off_label_cbt = this->attributes.off_label.subscribe([&](auto...) {
            ++global_counter<"selection_widget:off_label:constrain">;
            process_event({gui_event_type::window_reconstrain});
        });

        _delegate_cbt = this->delegate->subscribe([&] {
            update_options();
        });
        _delegate_cbt();

        this->delegate->init(*this);
    }

    /** Construct a selection widget which will monitor an option list and a
     * value.
     *
     * @param parent The owner of the selection widget.
     * @param value The value or observer value to monitor.
     * @param option_list An vector or an observer vector of pairs of keys and
     *                    labels. The keys are of the same type as the @a value.
     *                    The labels are of type `label`.
     * @param attributes Different attributes used to configure the label's on the selection box:
     *                   a `label`, `alignment` or `semantic_text_style`. If an label is passed
     *                   it is used as the label to show in the off-state.
     */
    template<
        incompatible_with<attributes_type> Value,
        forward_of<observer<std::vector<std::pair<observer_decay_t<Value>, label>>>> OptionList,
        selection_widget_attribute... Attributes>
    selection_widget(
        widget *parent,
        Value&& value,
        OptionList&& option_list,
        Attributes&&...attributes) noexcept requires requires
    {
        make_default_delegate(std::forward<Value>(value), std::forward<OptionList>(option_list));
        attributes_type{std::forward<Attributes>(attributes)...};
    } :
        selection_widget(
            parent,
            attributes_type{std::forward<Attributes>(attributes)...},
            make_default_delegate(std::forward<Value>(value), std::forward<OptionList>(option_list)))
    {
    }

    /// @privatesection
    [[nodiscard]] generator<widget_intf&> children(bool include_invisible) noexcept override
    {
        co_yield *_overlay_widget;
        co_yield *_current_label_widget;
        co_yield *_off_label_widget;
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        hi_assert_not_null(_off_label_widget);
        hi_assert_not_null(_current_label_widget);
        hi_assert_not_null(_overlay_widget);

        _layout = {};
        _off_label_constraints = _off_label_widget->update_constraints();
        _current_label_constraints = _current_label_widget->update_constraints();
        _overlay_constraints = _overlay_widget->update_constraints();

        hilet extra_size = extent2{theme().size() + theme().margin<float>() * 2.0f, theme().margin<float>() * 2.0f};

        auto r = max(_off_label_constraints + extra_size, _current_label_constraints + extra_size);

        // Make it so that the scroll widget can scroll vertically.
        _scroll_widget->minimum.copy()->height() = theme().size();

        r.minimum.width() = std::max(r.minimum.width(), _overlay_constraints.minimum.width() + extra_size.width());
        r.preferred.width() = std::max(r.preferred.width(), _overlay_constraints.preferred.width() + extra_size.width());
        r.maximum.width() = std::max(r.maximum.width(), _overlay_constraints.maximum.width() + extra_size.width());
        r.margins = theme().margin();
        r.padding = theme().margin();
        r.alignment = resolve(*attributes.alignment, os_settings::left_to_right());
        hi_axiom(r.holds_invariant());
        return r;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            if (os_settings::left_to_right()) {
                _left_box_rectangle = aarectangle{0.0f, 0.0f, theme().size(), context.height()};

                // The unknown_label is located to the right of the selection box icon.
                hilet option_rectangle = aarectangle{
                    _left_box_rectangle.right() + theme().margin<float>(),
                    0.0f,
                    context.width() - _left_box_rectangle.width() - theme().margin<float>() * 2.0f,
                    context.height()};
                _off_label_shape = box_shape{_off_label_constraints, option_rectangle, theme().baseline_adjustment()};
                _current_label_shape = box_shape{_off_label_constraints, option_rectangle, theme().baseline_adjustment()};

            } else {
                _left_box_rectangle = aarectangle{context.width() - theme().size(), 0.0f, theme().size(), context.height()};

                // The unknown_label is located to the left of the selection box icon.
                hilet option_rectangle = aarectangle{
                    theme().margin<float>(),
                    0.0f,
                    context.width() - _left_box_rectangle.width() - theme().margin<float>() * 2.0f,
                    context.height()};
                _off_label_shape = box_shape{_off_label_constraints, option_rectangle, theme().baseline_adjustment()};
                _current_label_shape = box_shape{_off_label_constraints, option_rectangle, theme().baseline_adjustment()};
            }

            _chevrons_glyph = find_glyph(elusive_icon::ChevronUp);
            hilet chevrons_glyph_bbox = _chevrons_glyph.get_metrics().bounding_rectangle * theme().icon_size();
            _chevrons_rectangle = align(_left_box_rectangle, chevrons_glyph_bbox, alignment::middle_center());
        }

        // The overlay itself will make sure the overlay fits the window, so we give the preferred size and position
        // from the point of view of the selection widget.
        // The overlay should start on the same left edge as the selection box and the same width.
        // The height of the overlay should be the maximum height, which will show all the options.
        hilet overlay_width = std::clamp(
            context.width() - theme().size(), _overlay_constraints.minimum.width(), _overlay_constraints.maximum.width());
        hilet overlay_height = _overlay_constraints.preferred.height();
        hilet overlay_x = os_settings::left_to_right() ? theme().size() : context.width() - theme().size() - overlay_width;
        hilet overlay_y = (context.height() - overlay_height) / 2;
        hilet overlay_rectangle_request = aarectangle{overlay_x, overlay_y, overlay_width, overlay_height};
        hilet overlay_rectangle = make_overlay_rectangle(overlay_rectangle_request);
        _overlay_shape = box_shape{_overlay_constraints, overlay_rectangle, theme().baseline_adjustment()};
        _overlay_widget->set_layout(context.transform(_overlay_shape, 20.0f));

        _off_label_widget->set_layout(context.transform(_off_label_shape));
        _current_label_widget->set_layout(context.transform(_current_label_shape));
    }

    void draw(draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible) {
            if (overlaps(context, layout())) {
                draw_outline(context);
                draw_left_box(context);
                draw_chevrons(context);

                _off_label_widget->draw(context);
                _current_label_widget->draw(context);
            }

            // Overlay is outside of the overlap of the selection widget.
            _overlay_widget->draw(context);
        }
    }

    bool handle_event(gui_event const& event) noexcept override
    {
        switch (event.type()) {
        case gui_event_type::mouse_up:
            if (*mode >= widget_mode::partial and _has_options and layout().rectangle().contains(event.mouse().position)) {
                return handle_event(gui_event_type::gui_activate);
            }
            return true;

        case gui_event_type::gui_activate_next:
            // Handle gui_active_next so that the next widget will NOT get keyboard focus.
            // The previously selected item needs the get keyboard focus instead.
        case gui_event_type::gui_activate:
            if (*mode >= widget_mode::partial and _has_options and not _selecting) {
                start_selecting();
            } else {
                stop_selecting();
            }
            ++global_counter<"selection_widget:gui_activate:relayout">;
            process_event({gui_event_type::window_relayout});
            return true;

        case gui_event_type::gui_cancel:
            if (*mode >= widget_mode::partial and _has_options and _selecting) {
                stop_selecting();
            }
            ++global_counter<"selection_widget:gui_cancel:relayout">;
            process_event({gui_event_type::window_relayout});
            return true;

        default:;
        }

        return super::handle_event(event);
    }

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (*mode >= widget_mode::partial) {
            auto r = _overlay_widget->hitbox_test_from_parent(position);

            if (layout().contains(position)) {
                r = std::max(r, hitbox{id, _layout.elevation, _has_options ? hitbox_type::button : hitbox_type::_default});
            }

            return r;
        } else {
            return {};
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        hi_axiom(loop::main().on_thread());
        return *mode >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::normal) and _has_options;
    }

    [[nodiscard]] color focus_color() const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (*mode >= widget_mode::partial and _has_options and _selecting) {
            return theme().color(semantic_color::accent);
        } else {
            return super::focus_color();
        }
    }

    /// @endprivatesection
private:
    bool _notification_from_delegate = true;

    std::unique_ptr<label_widget> _current_label_widget;
    box_constraints _current_label_constraints;
    box_shape _current_label_shape;

    std::unique_ptr<label_widget> _off_label_widget;
    box_constraints _off_label_constraints;
    box_shape _off_label_shape;

    aarectangle _left_box_rectangle;

    font_book::font_glyph_type _chevrons_glyph;
    aarectangle _chevrons_rectangle;

    bool _selecting = false;
    bool _has_options = false;

    std::unique_ptr<overlay_widget> _overlay_widget;
    box_constraints _overlay_constraints;
    box_shape _overlay_shape;

    vertical_scroll_widget *_scroll_widget = nullptr;
    grid_widget *_grid_widget = nullptr;

    callback<void()> _delegate_cbt;
    callback<void(label)> _off_label_cbt;
    std::vector<callback<void()>> _menu_button_callbacks;

    void start_selecting() noexcept
    {
        hi_axiom(loop::main().on_thread());

        if (auto focus_id = delegate->keyboard_focus_id(*this)) {
            _selecting = true;
            _overlay_widget->mode = widget_mode::enabled;
            process_event(gui_event::window_set_keyboard_target(*focus_id, keyboard_focus_group::menu));
        }

        request_redraw();
    }

    void stop_selecting() noexcept
    {
        hi_axiom(loop::main().on_thread());
        _selecting = false;
        _overlay_widget->mode = widget_mode::invisible;
        request_redraw();
    }

    void update_options() noexcept
    {
        if (auto new_buttons = delegate->make_button_widgets(*this)) {
            _grid_widget->clear();
            _menu_button_callbacks.clear();

            for (auto &new_button : *new_buttons) {
                auto &new_button_ref = _grid_widget->push_bottom(std::move(new_button));

                _menu_button_callbacks.push_back(new_button_ref.subscribe(
                    [&] {
                        stop_selecting();
                    },
                    callback_flags::main));
            }
            
            ++global_counter<"selection_widget:update_options:constrain">;
            process_event({gui_event_type::window_reconstrain});
       } else {
            // The options have not changed.
        }

        if (auto selected_label = delegate->selected(*this)) {
            _off_label_widget->mode = widget_mode::invisible;
            _current_label_widget->label = *selected_label;
            _current_label_widget->mode = widget_mode::display;

        } else {
            _off_label_widget->mode = widget_mode::display;
            _current_label_widget->mode = widget_mode::invisible;
        }
    }

    void draw_outline(draw_context const& context) noexcept
    {
        context.draw_box(
            layout(),
            layout().rectangle(),
            background_color(),
            focus_color(),
            theme().border_width(),
            border_side::inside,
            theme().rounding_radius());
    }

    void draw_left_box(draw_context const& context) noexcept
    {
        hilet corner_radii = os_settings::left_to_right() ?
            hi::corner_radii(theme().rounding_radius<float>(), 0.0f, theme().rounding_radius<float>(), 0.0f) :
            hi::corner_radii(0.0f, theme().rounding_radius<float>(), 0.0f, theme().rounding_radius<float>());
        context.draw_box(layout(), translate_z(0.1f) * _left_box_rectangle, focus_color(), corner_radii);
    }

    void draw_chevrons(draw_context const& context) noexcept
    {
        context.draw_glyph(layout(), translate_z(0.2f) * _chevrons_rectangle, _chevrons_glyph, label_color());
    }
};

}} // namespace hi::v1
