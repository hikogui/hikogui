// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/text_field_widget.hpp Defines text_field_widget.
 * @ingroup widgets
 */

#pragma once

#include "text_field_delegate.hpp"
#include "widget.hpp"
#include "label_widget.hpp"
#include "scroll_widget.hpp"
#include "../l10n/l10n.hpp"
#include "../macros.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>
#include <coroutine>

hi_export_module(hikogui.widgets.text_field_widget);

hi_export namespace hi { inline namespace v1 {

template<typename Context>
concept text_field_widget_attribute = text_widget_attribute<Context>;

/** A single line text field.
 *
 * A text field has the following visual elements:
 *  - A text field box which surrounds the user-editable text.
 *    It will use a color to show when the text-field has keyboard focus.
 *    It will use another color to show when the editable text is incorrect.
 *    Inside this box are the following elements:
 *     + Prefix: an icon describing the meaning, such as a search icon, or password, or popup-chevron.
 *     + Editable text
 *     + Suffix: text that follows the editable text, such as a SI base units like " kg" or " Hz".
 *  - Outside the text field box is an optional error message.
 *  - A popup window can be used to select between suggestions.
 *
 * Two commit modes:
 *  - on-activate: When pressing enter or changing keyboard focus using tab or clicking in another
 *                 field; as long as the text value can be validly converted.
 *                 The text will be converted to the observed object and committed.
 *                 When pressing escape, the text reverts to the observed object value.
 *  - continues: Every change of the text value of the input value is immediately converted and committed
 *               to the observed object; as long as the text value can be validly converted.
 *
 * The observed object needs to be convertible to and from a string using to_string() and from_string().
 * If from_string() throws a parse_error() its message will be displayed next to the text field.
 *
 * A custom validate function can be passed to validate the string and display a message next to the
 * text field.
 *
 * A custom transform function can be used to filter text on a modification-by-modification basis.
 * The filter takes the previous text and the new text after modification and returns the text that
 * should be shown in the field. This allows the filter to reject certain characters or limit the size.
 *
 * The maximum width of the text field is defined in the number of EM of the current selected font.
 *
 * @ingroup widgets
 */
class text_field_widget : public widget {
public:
    using delegate_type = text_field_delegate;
    using super = widget;

    std::shared_ptr<delegate_type> delegate;

    /** Continues update mode.
     * If true then the value will update on every edit of the text field.
     */
    observer<bool> continues = false;

    /** The style of the text.
     */
    observer<semantic_text_style> text_style = semantic_text_style::label;

    /** The alignment of the text.
     */
    observer<alignment> alignment = alignment::middle_flush();

    virtual ~text_field_widget()
    {
        hi_assert_not_null(delegate);
        delegate->deinit(*this);
    }

    text_field_widget(widget_intf const* parent, std::shared_ptr<delegate_type> delegate) noexcept :
        super(parent), delegate(std::move(delegate)), _text()
    {
        hi_assert_not_null(this->delegate);
        _delegate_cbt = this->delegate->subscribe([&] {
            ++global_counter<"text_field_widget:delegate:layout">;
            process_event({gui_event_type::window_relayout});
        });
        this->delegate->init(*this);

        _scroll_widget = std::make_unique<scroll_widget<axis::none>>(this);
        _text_widget = &_scroll_widget->emplace<text_widget>(_text, alignment, text_style);
        _text_widget->set_mode(widget_mode::partial);

        _error_label_widget =
            std::make_unique<label_widget>(this, _error_label, alignment::top_left(), semantic_text_style::error);

        _continues_cbt = continues.subscribe([&](auto...) {
            ++global_counter<"text_field_widget:continues:constrain">;
            process_event({gui_event_type::window_reconstrain});
        });
        _text_style_cbt = text_style.subscribe([&](auto...) {
            ++global_counter<"text_field_widget:text_style:constrain">;
            process_event({gui_event_type::window_reconstrain});
        });
        _text_cbt = _text.subscribe([&](auto...) {
            ++global_counter<"text_field_widget:text:constrain">;
            process_event({gui_event_type::window_reconstrain});
        });
        _error_label_cbt = _error_label.subscribe([&](auto const& new_value) {
            ++global_counter<"text_field_widget:error_label:constrain">;
            process_event({gui_event_type::window_reconstrain});
        });
    }

    template<text_field_widget_attribute... Attributes>
    text_field_widget(
        widget_intf const* parent,
        std::shared_ptr<delegate_type> delegate,
        Attributes&&...attributes) noexcept :
        text_field_widget(parent, std::move(delegate))
    {
        set_attributes(std::forward<Attributes>(attributes)...);
    }

    /** Construct a text field widget.
     *
     * @param parent The owner of this widget.
     * @param value The value or `observer` value which represents the state of the text-field.
     * @param attributes A set of attributes used to configure the text widget: a `alignment` or `semantic_text_style`.
     */
    template<incompatible_with<std::shared_ptr<delegate_type>> Value, text_field_widget_attribute... Attributes>
    text_field_widget(
        widget_intf const* parent,
        Value&& value,
        Attributes&&...attributes) noexcept requires requires
    {
        make_default_text_field_delegate(std::forward<Value>(value));
    } : text_field_widget(parent, make_default_text_field_delegate(std::forward<Value>(value)), std::forward<Attributes>(attributes)...) {}

    /// @privatesection
    [[nodiscard]] generator<widget_intf&> children(bool include_invisible) noexcept override
    {
        co_yield *_scroll_widget;
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        hi_assert_not_null(delegate);
        hi_assert_not_null(_error_label_widget);
        hi_assert_not_null(_scroll_widget);

        if (_text_widget->focus()) {
            // Update the optional error value from the string conversion when the text-widget has keyboard focus.
            _error_label = delegate->validate(*this, *_text);

        } else {
            // When field is not focused, simply follow the observed_value.
            revert(false);
        }

        _layout = {};
        _scroll_constraints = _scroll_widget->update_constraints();

        auto const scroll_width = 100;
        auto const box_size = extent2{
            _scroll_constraints.margins.left() + scroll_width + _scroll_constraints.margins.right(),
            _scroll_constraints.margins.top() + _scroll_constraints.preferred.height() + _scroll_constraints.margins.bottom()};

        auto size = box_size;
        auto margins = theme().margin();
        if (_error_label->empty()) {
            _error_label_widget->set_mode(widget_mode::invisible);
            _error_label_constraints = _error_label_widget->update_constraints();

        } else {
            _error_label_widget->set_mode(widget_mode::display);
            _error_label_constraints = _error_label_widget->update_constraints();
            inplace_max(size.width(), _error_label_constraints.preferred.width());
            size.height() += _error_label_constraints.margins.top() + _error_label_constraints.preferred.height();
            inplace_max(margins.left(), _error_label_constraints.margins.left());
            inplace_max(margins.right(), _error_label_constraints.margins.right());
            inplace_max(margins.bottom(), _error_label_constraints.margins.bottom());
        }

        // The alignment of a text-field is not based on the text-widget due to the intermediate scroll widget.
        auto const resolved_alignment = resolve_mirror(*alignment, os_settings::left_to_right());

        return {size, size, size, resolved_alignment, margins};
    }
    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            auto const scroll_size = extent2{
                context.width(),
                _scroll_constraints.margins.top() + _scroll_constraints.preferred.height() +
                    _scroll_constraints.margins.bottom()};

            auto const scroll_rectangle = aarectangle{point2{0, context.height() - scroll_size.height()}, scroll_size};
            _scroll_shape = box_shape{_scroll_constraints, scroll_rectangle, theme().baseline_adjustment()};

            if (_error_label_widget->mode() > widget_mode::invisible) {
                auto const error_label_rectangle =
                    aarectangle{0, 0, context.rectangle().width(), _error_label_constraints.preferred.height()};
                _error_label_shape = box_shape{_error_label_constraints, error_label_rectangle, theme().baseline_adjustment()};
            }
        }

        if (_error_label_widget->mode() > widget_mode::invisible) {
            _error_label_widget->set_layout(context.transform(_error_label_shape));
        }
        _scroll_widget->set_layout(context.transform(_scroll_shape));
    }
    void draw(draw_context const& context) noexcept override
    {
        if (mode() > widget_mode::invisible and overlaps(context, layout())) {
            draw_background_box(context);

            _scroll_widget->draw(context);
            _error_label_widget->draw(context);
        }
    }
    bool handle_event(gui_event const& event) noexcept override
    {
        switch (event.type()) {
        case gui_event_type::gui_cancel:
            if (mode() >= widget_mode::partial) {
                revert(true);
                return true;
            }
            break;

        case gui_event_type::gui_activate:
            if (mode() >= widget_mode::partial) {
                commit(true);
                return super::handle_event(event);
            }
            break;

        default:;
        }

        return super::handle_event(event);
    }
    hitbox hitbox_test(point2 position) const noexcept override
    {
        if (mode() >= widget_mode::partial) {
            auto r = hitbox{};
            r = _scroll_widget->hitbox_test_from_parent(position, r);
            r = _error_label_widget->hitbox_test_from_parent(position, r);
            return r;
        } else {
            return hitbox{};
        }
    }
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        if (mode() >= widget_mode::partial) {
            return _scroll_widget->accepts_keyboard_focus(group);
        } else {
            return false;
        }
    }
    [[nodiscard]] color focus_color() const noexcept override
    {
        if (mode() >= widget_mode::partial) {
            if (not _error_label->empty()) {
                return theme().text_style(semantic_text_style::error)->color;
            } else if (_text_widget->focus()) {
                return theme().accent_color();
            } else if (phase() == widget_phase::hover) {
                return theme().border_color(_layout.layer + 1);
            } else {
                return theme().border_color(_layout.layer);
            }

        } else {
            return theme().border_color(_layout.layer - 1);
        }
    }
    /// @endprivatesection
private:
    /** The scroll widget embeds the text widget.
     */
    std::unique_ptr<scroll_widget<axis::none>> _scroll_widget;
    box_constraints _scroll_constraints;
    box_shape _scroll_shape;

    /** The text widget inside the scroll widget.
     */
    text_widget *_text_widget = nullptr;

    /** The text edited by the _text_widget.
     */
    observer<gstring> _text;

    /** An error string to show to the user.
     */
    observer<label> _error_label;
    std::unique_ptr<label_widget> _error_label_widget;
    box_constraints _error_label_constraints;
    box_shape _error_label_shape;

    callback<void()> _delegate_cbt;
    callback<void(bool)> _continues_cbt;
    callback<void(semantic_text_style)> _text_style_cbt;
    callback<void(gstring)> _text_cbt;
    callback<void(label)> _error_label_cbt;

    void set_attributes() noexcept {}

    template<text_field_widget_attribute First, text_field_widget_attribute... Rest>
    void set_attributes(First&& first, Rest&&...rest) noexcept
    {
        if constexpr (forward_of<First, observer<hi::alignment>>) {
            alignment = std::forward<First>(first);
        } else if constexpr (forward_of<First, observer<hi::semantic_text_style>>) {
            text_style = std::forward<First>(first);
        } else {
            hi_static_no_default();
        }

        set_attributes(std::forward<Rest>(rest)...);
    }

    void revert(bool force) noexcept
    {
        hi_assert_not_null(delegate);
        _text = delegate->text(*this);
        _error_label = label{};
    }
    void commit(bool force) noexcept
    {
        hi_axiom(loop::main().on_thread());
        hi_assert_not_null(delegate);

        if (*continues or force) {
            auto text = *_text;

            if (delegate->validate(*this, text).empty()) {
                // text is valid.
                delegate->set_text(*this, text);
            }

            // After commit get the canonical text to display from the delegate.
            _text = delegate->text(*this);
            _error_label = label{};
        }
    }
    void draw_background_box(draw_context const& context) const noexcept
    {
        auto const outline = _scroll_shape.rectangle;

        auto const corner_radii = hi::corner_radii(0.0f, 0.0f, theme().rounding_radius<float>(), theme().rounding_radius<float>());
        context.draw_box(layout(), outline, background_color(), corner_radii);

        auto const line = line_segment(get<0>(outline), get<1>(outline));
        context.draw_line(layout(), translate3{0.0f, 0.5f, 0.1f} * line, theme().border_width(), focus_color());
    }
};

}} // namespace hi::v1
