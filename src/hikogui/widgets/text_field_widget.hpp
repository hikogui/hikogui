// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/text_field_widget.hpp Defines text_field_widget.
 * @ingroup widgets
 */

#pragma once

#include "text_field_delegate.hpp"
#include "label_widget.hpp"
#include "scroll_widget.hpp"
#include "../label.hpp"
#include "../GUI/module.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace hi { inline namespace v1 {

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
template<fixed_string Name = "">
class text_field_widget final : public widget {
public:
    using delegate_type = text_field_delegate;
    using super = widget;
    constexpr static auto prefix = Name / "text-field";

    std::shared_ptr<delegate_type> delegate;

    /** Continues update mode.
     * If true then the value will update on every edit of the text field.
     */
    observer<bool> continues = false;

    /** The alignment of the text.
     */
    observer<alignment> alignment = alignment::middle_flush();

    virtual ~text_field_widget()
    {
        hi_assert_not_null(delegate);
        delegate->deinit(*this);
    }

    text_field_widget(widget *parent, std::shared_ptr<delegate_type> delegate) noexcept :
        super(parent), delegate(std::move(delegate)), _text()
    {
        hi_assert_not_null(this->delegate);
        _delegate_cbt = this->delegate->subscribe([&] {
            ++global_counter<"text_field_widget:delegate:layout">;
            process_event({gui_event_type::window_relayout});
        });
        this->delegate->init(*this);

        _scroll_widget = std::make_unique<scroll_widget<axis::none, prefix>>(this);
        _text_widget = &_scroll_widget->make_widget<text_widget<prefix>>(_text, alignment);
        _text_widget->mode = widget_mode::partial;

        // XXX The error-label should use the text_phrasing::error.
        _error_label_widget = std::make_unique<label_widget<prefix / "error">>(this, _error_label, alignment::top_left());

        _continues_cbt = continues.subscribe([&](auto...) {
            ++global_counter<"text_field_widget:continues:constrain">;
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

    text_field_widget(
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        text_field_widget_attribute auto&&...attributes) noexcept :
        text_field_widget(parent, std::move(delegate))
    {
        set_attributes(hi_forward(attributes)...);
    }

    /** Construct a text field widget.
     *
     * @param parent The owner of this widget.
     * @param value The value or `observer` value which represents the state of the text-field.
     * @param attributes A set of attributes used to configure the text widget: a `alignment`.
     */
    text_field_widget(
        widget *parent,
        different_from<std::shared_ptr<delegate_type>> auto&& value,
        text_field_widget_attribute auto&&...attributes) noexcept
        requires requires { make_default_text_field_delegate(hi_forward(value)); }
        : text_field_widget(parent, make_default_text_field_delegate(hi_forward(value)), hi_forward(attributes)...)
    {
    }

    /// @privatesection
    [[nodiscard]] generator<widget const&> children(bool include_invisible) const noexcept override
    {
        co_yield *_scroll_widget;
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        hi_assert_not_null(delegate);
        hi_assert_not_null(_error_label_widget);
        hi_assert_not_null(_scroll_widget);

        if (*_text_widget->focus) {
            // Update the optional error value from the string conversion when
            // the text-widget has keyboard focus.
            if (auto error_label = delegate->validate(*this, *_text)) {
                _error_label = *error_label;
            } else {
                // Error label is used by a widget and can not be an optional,
                // we use an empty string to denote no-error.
                _error_label = label{};
            }

        } else {
            // When field is not focused, simply follow the observed_value.
            revert(false);
        }

        _scroll_constraints = _scroll_widget->update_constraints();

        hilet scroll_width = 100;
        hilet box_size = extent2i{
            _scroll_constraints.margins.left() + scroll_width + _scroll_constraints.margins.right(),
            _scroll_constraints.margins.top() + _scroll_constraints.preferred.height() + _scroll_constraints.margins.bottom()};

        auto size = box_size;
        auto margins = theme<prefix>.margin(this);
        if (_error_label->empty()) {
            _error_label_widget->mode = widget_mode::invisible;
            _error_label_constraints = _error_label_widget->update_constraints();

        } else {
            _error_label_widget->mode = widget_mode::display;
            _error_label_constraints = _error_label_widget->update_constraints();
            inplace_max(size.width(), _error_label_constraints.preferred.width());
            size.height() += _error_label_constraints.margins.top() + _error_label_constraints.preferred.height();
            inplace_max(margins.left(), _error_label_constraints.margins.left());
            inplace_max(margins.right(), _error_label_constraints.margins.right());
            inplace_max(margins.bottom(), _error_label_constraints.margins.bottom());
        }

        // The alignment of a text-field is not based on the text-widget due to the intermediate scroll widget.
        hilet resolved_alignment = resolve_mirror(*alignment, os_settings::left_to_right());

        return {size, size, size, resolved_alignment, margins};
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(layout, context)) {
            hilet scroll_size = extent2i{
                context.width(),
                _scroll_constraints.margins.top() + _scroll_constraints.preferred.height() +
                    _scroll_constraints.margins.bottom()};

            hilet scroll_rectangle = aarectanglei{point2i{0, context.height() - scroll_size.height()}, scroll_size};
            _scroll_shape = box_shape{_scroll_constraints, scroll_rectangle, theme<prefix>.cap_height(this)};

            if (*_error_label_widget->mode > widget_mode::invisible) {
                hilet error_label_rectangle =
                    aarectanglei{0, 0, context.rectangle().width(), _error_label_constraints.preferred.height()};
                _error_label_shape = box_shape{_error_label_constraints, error_label_rectangle, theme<prefix>.cap_height(this)};
            }
        }

        if (*_error_label_widget->mode > widget_mode::invisible) {
            _error_label_widget->set_layout(context.transform(_error_label_shape));
        }
        _scroll_widget->set_layout(context.transform(_scroll_shape));
    }

    void draw(widget_draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout)) {
            draw_background_box(context);

            _scroll_widget->draw(context);
            _error_label_widget->draw(context);
        }
    }

    bool handle_event(gui_event const& event) noexcept override
    {
        switch (event.type()) {
        case gui_event_type::gui_cancel:
            if (*mode >= widget_mode::partial) {
                revert(true);
                return true;
            }
            break;

        case gui_event_type::gui_activate:
            if (*mode >= widget_mode::partial) {
                commit(true);
                return super::handle_event(event);
            }
            break;

        default:;
        }

        return super::handle_event(event);
    }

    hitbox hitbox_test(point2i position) const noexcept override
    {
        if (*mode >= widget_mode::partial) {
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
        if (*mode >= widget_mode::partial) {
            return _scroll_widget->accepts_keyboard_focus(group);
        } else {
            return false;
        }
    }
    /// @endprivatesection
private:
    notifier<>::callback_token _delegate_cbt;

    /** The scroll widget embeds the text widget.
     */
    std::unique_ptr<scroll_widget<axis::none, prefix>> _scroll_widget;
    box_constraints _scroll_constraints;
    box_shape _scroll_shape;

    /** The text widget inside the scroll widget.
     */
    text_widget<prefix> *_text_widget = nullptr;

    /** The text edited by the _text_widget.
     */
    observer<hi::text> _text;

    /** An error string to show to the user.
     */
    observer<label> _error_label;
    std::unique_ptr<label_widget<join_path(prefix, "error")>> _error_label_widget;
    box_constraints _error_label_constraints;
    box_shape _error_label_shape;

    typename decltype(continues)::callback_token _continues_cbt;
    typename decltype(_text)::callback_token _text_cbt;
    typename decltype(_error_label)::callback_token _error_label_cbt;

    void set_attributes() noexcept {}
    void set_attributes(text_field_widget_attribute auto&& first, text_field_widget_attribute auto&&...rest) noexcept
    {
        if constexpr (forward_of<decltype(first), observer<hi::alignment>>) {
            alignment = hi_forward(first);
        } else {
            hi_static_no_default();
        }

        set_attributes(hi_forward(rest)...);
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
            if (not delegate->validate(*this, *_text)) {
                // text is valid.
                delegate->set_text(*this, *_text);
            }

            // After commit get the canonical text to display from the delegate.
            _text = delegate->text(*this);
            _error_label = label{};
        }
    }

    void draw_background_box(widget_draw_context const& context) const noexcept
    {
        hilet outline = narrow_cast<aarectangle>(_scroll_shape.rectangle);

        context.draw_box(layout, outline, theme<prefix>.background_color(this), theme<prefix>.border_radius(this));

        // A text field has a line under the box, which changes color on error.
        hilet line = line_segment(get<0>(outline), get<1>(outline));

        hilet outline_color = _error_label->empty() ? theme<prefix>.border_color(this) : color::red();
        context.draw_line(layout, translate3{0.0f, 0.5f, 0.1f} * line, theme<prefix>.border_width(this), outline_color);
    }
};

}} // namespace hi::v1
