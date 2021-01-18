// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "text_field_delegate.hpp"
#include "widget.hpp"
#include "../text/EditableText.hpp"
#include "../format.hpp"
#include "../label.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

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
 */
template<typename T>
class text_field_widget final : public widget {
public:
    using value_type = T;
    using delegate_type = text_field_delegate<value_type>;
    using super = widget;

    observable<value_type> value;

    template<typename Value = observable<value_type>>
    text_field_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::weak_ptr<delegate_type> delegate,
        Value &&value = {}) noexcept :
        super(window, parent),
        value(std::forward<Value>(value)),
        _delegate(delegate),
        _field(theme::global->labelStyle),
        _shaped_text()
    {
        _value_callback = this->value.subscribe([this](auto...) {
            ttlet lock = std::scoped_lock(gui_system_mutex);
            _request_relayout = true;
        });
    }

    template<typename Value = observable<value_type>>
    text_field_widget(gui_window &window, std::shared_ptr<widget> parent, Value &&value = {}) noexcept :
        text_field_widget(window, parent, text_field_delegate_default<value_type>(), std::forward<Value>(value))
    {
    }

    ~text_field_widget() {}

    /** Set the delegate
     * The delegate is used to convert between the value type and the string the user sees and enters.
     *
     * @param delegate The delegate for this text field
     */
    void set_delegate(std::weak_ptr<delegate_type> &&delegate) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        _delegate = delegate;
    }

    /** Set or unset continues mode.
     * @param flag If true then the value will update on every edit of the text field.
     *             If false then the value will only update when the focus changes.
     */
    void set_continues(bool flag) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        _continues = flag;
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            ttlet text_style = theme::global->labelStyle;
            ttlet text_font_id = font_book::global->find_font(text_style.family_id, text_style.variant);
            ttlet &text_font = font_book::global->get_font(text_font_id);
            ttlet text_digit_width = text_font.description.DigitWidth * text_style.scaled_size();

            if (auto delegate = _delegate.lock()) {
                _text_width = std::ceil(text_digit_width * narrow_cast<float>(delegate->text_width(*this)));
            } else {
                _text_width = 100.0;
            }

            _preferred_size = {
                f32x4{_text_width + theme::global->margin * 2.0f, theme::global->smallSize + theme::global->margin * 2.0f},
                f32x4{std::numeric_limits<float>::infinity(), theme::global->smallSize + theme::global->margin * 2.0f}};
            _preferred_base_line = relative_base_line{vertical_alignment::middle, 0.0f, 200.0f};
            _width_resistance = 2;

            return true;
        } else {
            return false;
        }
    }

    void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (_focus && display_time_point >= _next_redraw_time_point) {
            window.request_redraw(window_clipping_rectangle());
        }

        need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            _text_field_rectangle =
                aarect{rectangle().x(), rectangle().y(), _text_width + theme::global->margin * 2.0f, rectangle().height()};
            auto _text_field_window_rectangle = aarect{
                _window_rectangle.x() + theme::global->borderWidth * 2.0f,
                _window_rectangle.y(),
                _text_width + theme::global->margin * 2.0f - theme::global->borderWidth * 4.0f,
                _window_rectangle.height()};

            // Set the clipping rectangle to within the border of the input field.
            // Add another border width, so glyphs do not touch the border.
            _text_field_clipping_rectangle = intersect(window_clipping_rectangle(), _text_field_window_rectangle);

            _text_rectangle = shrink(_text_field_rectangle, theme::global->margin);

            ttlet field_str = static_cast<std::string>(_field);

            if (auto delegate = _delegate.lock()) {
                if (_focus) {
                    // Update the optional error value from the string conversion when the
                    // field has keyboard focus.
                    delegate->from_string(*this, field_str, _error);

                } else {
                    // When field is not focused, simply follow the observed_value.
                    _field = delegate->to_string(*this, *value);
                    _error = {};
                }

            } else {
                _field = {};
                _error = l10n("system error: delegate missing");
            }

            _field.setStyleOfAll(theme::global->labelStyle);
            _field.setWidth(std::numeric_limits<float>::infinity());
            _shaped_text = _field.shapedText();

            // Record the last time the text is modified, so that the caret remains lit.
            _last_update_time_point = display_time_point;
        }

        super::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        _next_redraw_time_point = display_time_point + _blink_interval;

        if (overlaps(context, this->window_clipping_rectangle())) {
            scroll_text();

            draw_background_box(context);

            // After drawing the border around the input field make sure any other
            // drawing remains inside this border. And change the transform to account
            // for how much the text has scrolled.
            context.clipping_rectangle = _text_field_clipping_rectangle;
            context.transform = (mat::T(0.0, 0.0, 0.1f) * _text_translate) * context.transform;

            draw_selection_rectangles(context);
            draw_partial_grapheme_caret(context);
            draw_caret(context, display_time_point);
            draw_text(context);
        }

        super::draw(std::move(context), display_time_point);
    }

    bool handle_command(command command) noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        auto handled = super::handle_command(command);

        LOG_DEBUG("text_field_widget: Received command: {}", command);
        if (*enabled) {
            switch (command) {
            case command::text_edit_paste:
                handled = true;
                _field.handlePaste(window.getTextFromClipboard());
                commit(false);
                break;

            case command::text_edit_copy:
                handled = true;
                window.setTextOnClipboard(_field.handleCopy());
                break;

            case command::text_edit_cut:
                handled = true;
                window.setTextOnClipboard(_field.handleCut());
                break;

            default: handled |= _field.handle_command(command); commit(false);
            }
        }

        _request_relayout = true;
        return handled;
    }

    bool handle_mouse_event(MouseEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        auto handled = super::handle_mouse_event(event);

        // Make sure we only scroll when dragging outside the widget.
        ttlet position = _from_window_transform * event.position;
        _drag_scroll_speed_x = 0.0f;
        _drag_click_count = event.clickCount;
        _drag_select_position = position;

        if (event.cause.leftButton) {
            handled = true;

            if (!*enabled) {
                return true;
            }

            switch (event.type) {
                using enum MouseEvent::Type;
            case ButtonDown:
                if (_text_rectangle.contains(position)) {
                    ttlet mouseInTextPosition = _text_inv_translate * position;

                    switch (event.clickCount) {
                    case 1:
                        if (event.down.shiftKey) {
                            _field.dragCursorAtCoordinate(mouseInTextPosition);
                        } else {
                            _field.setCursorAtCoordinate(mouseInTextPosition);
                        }
                        break;
                    case 2: _field.selectWordAtCoordinate(mouseInTextPosition); break;
                    case 3: _field.selectParagraphAtCoordinate(mouseInTextPosition); break;
                    default:;
                    }

                    // Record the last time the cursor is moved, so that the caret remains lit.
                    _last_update_time_point = event.timePoint;

                    window.request_redraw(window_clipping_rectangle());
                }
                break;

            case Drag:
                // When the mouse is dragged beyond the line input,
                // start scrolling the text and select on the edge of the textRectangle.
                if (position.x() > _text_rectangle.p3().x()) {
                    // The mouse is on the right of the text.
                    _drag_select_position.x() = _text_rectangle.p3().x();

                    // Scroll text to the left in points per second.
                    _drag_scroll_speed_x = 50.0f;

                } else if (position.x() < _text_rectangle.x()) {
                    // The mouse is on the left of the text.
                    _drag_select_position.x() = _text_rectangle.x();

                    // Scroll text to the right in points per second.
                    _drag_scroll_speed_x = -50.0f;
                }

                drag_select();

                window.request_redraw(window_clipping_rectangle());
                break;

            default:;
            }
        }
        return handled;
    }

    bool handle_keyboard_event(KeyboardEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        auto handled = super::handle_keyboard_event(event);

        if (*enabled) {
            switch (event.type) {
                using enum KeyboardEvent::Type;

            case Exited:
                // Do not modify `handled`; continue further processing of exit event.
                commit(true);
                break;

            case Grapheme:
                handled = true;
                _field.insertGrapheme(event.grapheme);
                commit(false);
                break;

            case PartialGrapheme:
                handled = true;
                _field.insertPartialGrapheme(event.grapheme);
                commit(false);
                break;

            default:;
            }
        }

        _request_relayout = true;
        return handled;
    }

    HitBox hitbox_test(f32x4 window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        if (window_clipping_rectangle().contains(window_position)) {
            return HitBox{weak_from_this(), _draw_layer, *enabled ? HitBox::Type::TextEdit : HitBox::Type::Default};
        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return is_normal(group) && *enabled;
    }

private:
    typename decltype(value)::callback_ptr_type _value_callback;

    std::weak_ptr<delegate_type> _delegate;

    bool _continues = false;

    /** An error string to show to the user.
     */
    l10n _error;

    float _text_width = 0.0f;
    aarect _text_rectangle = {};

    aarect _text_field_rectangle;
    aarect _text_field_clipping_rectangle;

    EditableText _field;
    ShapedText _shaped_text;
    aarect _left_to_right_caret = {};

    /** Scroll speed in points per second.
     * This is used when dragging outside of the widget.
     */
    float _drag_scroll_speed_x = 0.0f;

    /** Number of mouse clicks that caused the drag.
     */
    int _drag_click_count = 0;

    f32x4 _drag_select_position = {};

    /** How much the text has scrolled in points.
     */
    float _text_scroll_x = 0.0f;

    mat::T2 _text_translate;
    mat::T2 _text_inv_translate;

    static constexpr hires_utc_clock::duration _blink_interval = 500ms;
    hires_utc_clock::time_point _next_redraw_time_point;
    hires_utc_clock::time_point _last_update_time_point;

    void commit(bool force) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        if (_continues || force) {
            if (auto delegate = _delegate.lock()) {
                auto optional_value = delegate->from_string(*this, static_cast<std::string>(_field), _error);
                if (optional_value) {
                    value = *optional_value;
                }
            }
        }
    }

    void drag_select() noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        ttlet mouseInTextPosition = _text_inv_translate * _drag_select_position;
        switch (_drag_click_count) {
        case 1: _field.dragCursorAtCoordinate(mouseInTextPosition); break;
        case 2: _field.dragWordAtCoordinate(mouseInTextPosition); break;
        case 3: _field.dragParagraphAtCoordinate(mouseInTextPosition); break;
        default:;
        }
    }

    void scroll_text() noexcept
    {
        if (_drag_scroll_speed_x != 0.0f) {
            _text_scroll_x += _drag_scroll_speed_x * (1.0f / 60.0f);
            drag_select();

            // Once we are scrolling, don't stop.
            window.request_redraw(window_clipping_rectangle());

        } else if (_drag_click_count == 0) {
            // The following is for scrolling based on keyboard input, ignore mouse drags.

            // Scroll the text a quarter width to the left until the cursor is within the width
            // of the text field
            if (_left_to_right_caret.x() - _text_scroll_x > _text_rectangle.width()) {
                _text_scroll_x = _left_to_right_caret.x() - _text_rectangle.width() * 0.75f;
            }

            // Scroll the text a quarter width to the right until the cursor is within the width
            // of the text field
            while (_left_to_right_caret.x() - _text_scroll_x < 0.0f) {
                _text_scroll_x = _left_to_right_caret.x() - _text_rectangle.width() * 0.25f;
            }
        }

        // cap how far we scroll.
        ttlet max_scroll_width = std::max(0.0f, _shaped_text.preferred_extent.width() - _text_rectangle.width());
        _text_scroll_x = std::clamp(_text_scroll_x, 0.0f, max_scroll_width);

        // Calculate how much we need to translate the text.
        _text_translate = mat::T2(-_text_scroll_x, 0.0f) * _shaped_text.TMiddle(f32x4{_text_rectangle.x(), base_line()});
        _text_inv_translate = ~_text_translate;
    }

    void draw_background_box(draw_context context) const noexcept
    {
        ttlet line_color = context.color;

        context.color = context.fill_color;
        context.corner_shapes = {0.0f, 0.0f, theme::global->roundingRadius, theme::global->roundingRadius};
        context.draw_box_with_border_inside(_text_field_rectangle);

        ttlet line_rectangle = aarect{_text_field_rectangle.p0(), f32x4{_text_field_rectangle.width(), context.line_width}};
        context.transform = context.transform * mat::T(0.0f, 0.0f, 0.1f);
        if (_error && window.active) {
            context.fill_color = theme::global->errorLabelStyle.color;
        } else {
            context.fill_color = line_color;
        }
        context.draw_filled_quad(line_rectangle);
    }

    void draw_selection_rectangles(draw_context context) const noexcept
    {
        ttlet selection_rectangles = _field.selectionRectangles();
        for (ttlet selection_rectangle : selection_rectangles) {
            context.fill_color = theme::global->textSelectColor;
            context.draw_filled_quad(selection_rectangle);
        }
    }

    void draw_partial_grapheme_caret(draw_context context) const noexcept
    {
        ttlet partial_grapheme_caret = _field.partialGraphemeCaret();
        if (partial_grapheme_caret) {
            context.fill_color = theme::global->incompleteGlyphColor;
            context.draw_filled_quad(partial_grapheme_caret);
        }
    }

    void draw_caret(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
    {
        // Display the caret and handle blinking.
        ttlet duration_since_last_update = display_time_point - _last_update_time_point;
        ttlet nr_half_blinks = static_cast<int64_t>(duration_since_last_update / _blink_interval);

        ttlet blink_is_on = nr_half_blinks % 2 == 0;
        _left_to_right_caret = _field.leftToRightCaret();
        if (_left_to_right_caret && blink_is_on && _focus && window.active) {
            context.fill_color = theme::global->cursorColor;
            context.draw_filled_quad(_left_to_right_caret);
        }
    }

    void draw_text(draw_context context) const noexcept
    {
        context.transform = mat::T(0.0f, 0.0f, 0.2f) * context.transform;
        context.draw_text(_shaped_text);
    }
};

} // namespace tt
