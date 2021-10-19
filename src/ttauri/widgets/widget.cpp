// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "widget.hpp"
#include "../GUI/gui_window.hpp"
#include "../GUI/gui_system.hpp"
#include "../scoped_buffer.hpp"
#include <ranges>

namespace tt {

widget::widget(gui_window &_window, widget *parent) noexcept :
    window(_window), parent(parent), draw_layer(0.0f), logical_layer(0), semantic_layer(0)
{
    tt_axiom(is_gui_thread());

    if (parent) {
        draw_layer = parent->draw_layer + 1.0f;
        logical_layer = parent->logical_layer + 1;
        semantic_layer = parent->semantic_layer + 1;
    }

    _redraw_callback = std::make_shared<std::function<void()>>([this] {
        request_redraw();
    });

    _relayout_callback = std::make_shared<std::function<void()>>([this] {
        request_relayout();
    });
    _reconstrain_callback = std::make_shared<std::function<void()>>([this] {
        request_reconstrain();
    });

    enabled.subscribe(_redraw_callback);
    visible.subscribe(_redraw_callback);

    _minimum_size = extent2::nan();
    _preferred_size = extent2::nan();
    _maximum_size = extent2::nan();
}

widget::~widget()
{
    // The window must remove references such as mouse and keyboard targets to
    // this widget when it is removed.
    window.widget_is_destructing(this);
}

[[nodiscard]] bool widget::is_gui_thread() const noexcept
{
    return window.is_gui_thread();
}

tt::theme const &widget::theme() const noexcept
{
    return window.gui.theme();
}

tt::font_book &widget::font_book() const noexcept
{
    return *window.gui.font_book;
}

[[nodiscard]] float widget::margin() const noexcept
{
    tt_axiom(is_gui_thread());
    return theme().margin;
}

[[nodiscard]] color widget::background_color() const noexcept
{
    if (enabled) {
        if (hover) {
            return theme().color(theme_color::fill, semantic_layer + 1);
        } else {
            return theme().color(theme_color::fill, semantic_layer);
        }
    } else {
        return theme().color(theme_color::fill, semantic_layer - 1);
    }
}

[[nodiscard]] color widget::foreground_color() const noexcept
{
    if (enabled) {
        if (hover) {
            return theme().color(theme_color::border, semantic_layer + 1);
        } else {
            return theme().color(theme_color::border, semantic_layer);
        }
    } else {
        return theme().color(theme_color::border, semantic_layer - 1);
    }
}

[[nodiscard]] color widget::focus_color() const noexcept
{
    if (enabled) {
        if (focus && window.active) {
            return theme().color(theme_color::accent);
        } else if (hover) {
            return theme().color(theme_color::border, semantic_layer + 1);
        } else {
            return theme().color(theme_color::border, semantic_layer);
        }
    } else {
        return theme().color(theme_color::border, semantic_layer - 1);
    }
}

[[nodiscard]] color widget::accent_color() const noexcept
{
    if (enabled) {
        if (window.active) {
            return theme().color(theme_color::accent);
        } else {
            return theme().color(theme_color::border, semantic_layer);
        }
    } else {
        return theme().color(theme_color::border, semantic_layer - 1);
    }
}

[[nodiscard]] color widget::label_color() const noexcept
{
    if (enabled) {
        return theme().text_style(theme_text_style::label).color;
    } else {
        return theme().color(theme_color::border, semantic_layer - 1);
    }
}

/** Minimum size.
 * The absolute minimum size of the widget.
 * A container will never reserve less space for the widget.
 * For windows this size becomes a hard limit for the minimum window size.
 */
[[nodiscard]] extent2 widget::minimum_size() const noexcept
{
    tt_axiom(is_gui_thread());
    return _minimum_size;
}

/** Preferred size.
 * The preferred size of a widget.
 * Containers will initialize their layout algorithm at this size
 * before growing or shrinking.
 * For scroll-views this size will be used in the scroll-direction.
 * For tab-views this is propagated.
 * For windows this size is used to set the initial window size.
 */
[[nodiscard]] extent2 widget::preferred_size() const noexcept
{
    tt_axiom(is_gui_thread());
    return _preferred_size;
}

/** Maximum size.
 * The maximum size of a widget.
 * Containers will try to not grow a widget beyond the maximum size,
 * but it may do so to satisfy the minimum constraint on a neighboring widget.
 * For windows the maximum size becomes a hard limit for the window size.
 */
[[nodiscard]] extent2 widget::maximum_size() const noexcept
{
    tt_axiom(is_gui_thread());
    return _maximum_size;
}

/** Set the location and size of the widget inside the window.
 *
 * The parent should call this `set_layout_paramters()` before this `updateLayout()`.
 *
 * If the parent's layout did not change, it does not need to call this `set_layout_parameters()`.
 * This way the parent does not need to keep a cache, recalculate or query the client for these
 * layout parameters for each frame.
 *
 * @pre `mutex` must be locked by current thread.
 */
void widget::set_layout_parameters(
    geo::transformer auto const &local_to_parent,
    extent2 size,
    aarectangle const &clipping_rectangle) noexcept
{
    tt_axiom(is_gui_thread());

    _parent_to_local = ~local_to_parent;
    if (parent) {
        _window_to_local = ~local_to_parent * parent->window_to_local();
    } else {
        _window_to_local = ~local_to_parent;
    }
    _clipping_rectangle = clipping_rectangle;
    _visible_rectangle = intersect(aarectangle{size}, clipping_rectangle);
}

void widget::set_layout_parameters_from_parent(
    aarectangle child_rectangle,
    aarectangle parent_clipping_rectangle,
    float draw_layer_delta) noexcept
{
    tt_axiom(is_gui_thread());

    ttlet child_translate = translate2{child_rectangle};
    ttlet child_size = child_rectangle.size();
    ttlet rectangle = aarectangle{child_size};
    ttlet child_clipping_rectangle = intersect(~child_translate * parent_clipping_rectangle, rectangle + margin());

    set_layout_parameters(translate_z(draw_layer_delta) * child_translate, child_size, child_clipping_rectangle);
}

void widget::set_layout_parameters_from_parent(aarectangle child_rectangle) noexcept
{
    tt_axiom(is_gui_thread());

    if (parent) {
        ttlet draw_layer_delta = draw_layer - parent->draw_layer;
        return set_layout_parameters_from_parent(child_rectangle, parent->clipping_rectangle(), draw_layer_delta);
    } else {
        return set_layout_parameters_from_parent(child_rectangle, child_rectangle, 0.0f);
    }
}

[[nodiscard]] matrix3 widget::parent_to_local() const noexcept
{
    tt_axiom(is_gui_thread());
    return _parent_to_local;
}

[[nodiscard]] matrix3 widget::window_to_local() const noexcept
{
    tt_axiom(is_gui_thread());
    return _window_to_local;
}

[[nodiscard]] extent2 widget::size() const noexcept
{
    tt_axiom(is_gui_thread());
    return _size;
}

[[nodiscard]] float widget::width() const noexcept
{
    tt_axiom(is_gui_thread());
    return _size.width();
}

[[nodiscard]] float widget::height() const noexcept
{
    tt_axiom(is_gui_thread());
    return _size.height();
}

/** Get the rectangle in local coordinates.
 *
 * @pre `mutex` must be locked by current thread.
 */
[[nodiscard]] aarectangle widget::rectangle() const noexcept
{
    tt_axiom(is_gui_thread());
    return aarectangle{_size};
}

/** Return the base-line where the text should be located.
 * @return Number of pixels from the bottom of the widget where the base-line is located.
 */
[[nodiscard]] float widget::base_line() const noexcept
{
    tt_axiom(is_gui_thread());
    return rectangle().middle();
}

[[nodiscard]] aarectangle widget::clipping_rectangle() const noexcept
{
    tt_axiom(is_gui_thread());
    return _clipping_rectangle;
}

[[nodiscard]] bool widget::constrain(utc_nanoseconds display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(is_gui_thread());

    need_reconstrain |= _reconstrain.exchange(false);

    auto buffer = pmr::scoped_buffer<256>{};
    for (auto *child : children(buffer.allocator())) {
        if (child) {
            tt_axiom(child->parent == this);
            need_reconstrain |= child->constrain(display_time_point, need_reconstrain);
        }
    }

    return need_reconstrain;
}

void widget::draw(draw_context context, utc_nanoseconds display_time_point) noexcept
{
    tt_axiom(is_gui_thread());

    context.set_clipping_rectangle(_clipping_rectangle);
    auto buffer = pmr::scoped_buffer<256>{};
    for (auto *child : children(buffer.allocator())) {
        if (child) {
            tt_axiom(child->parent == this);

            if (child->visible) {
                child->draw(child->parent_to_local() * context, display_time_point);
            }
        }
    }
}

void widget::request_redraw() const noexcept
{
    window.request_redraw(_bounding_rectangle);
}

void widget::request_relayout() noexcept
{
    window.request_relayout();
}

void widget::request_reconstrain() noexcept
{
    _reconstrain.store(true, std::memory_order::relaxed);
    window.request_reconstrain();
}

[[nodiscard]] bool widget::handle_event(std::vector<command> const &commands) noexcept
{
    tt_axiom(is_gui_thread());
    for (ttlet command : commands) {
        if (handle_event(command)) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] hitbox widget::hitbox_test(point2 position) const noexcept
{
    tt_axiom(is_gui_thread());

    auto r = hitbox{};

    auto buffer = pmr::scoped_buffer<256>{};
    for (auto *child : children(buffer.allocator())) {
        if (child) {
            tt_axiom(child->parent == this);
            if (child->visible) {
                r = std::max(r, child->hitbox_test(point2{child->parent_to_local() * position}));
            }
        }
    }
    return r;
}

bool widget::handle_event(command command) noexcept
{
    tt_axiom(is_gui_thread());

    switch (command) {
        using enum tt::command;
    case gui_keyboard_enter:
        focus = true;
        // When scrolling, include the margin, so that the widget is clear from the edge of the
        // scroll view's aperture.
        scroll_to_show();
        request_redraw();
        return true;

    case gui_keyboard_exit:
        focus = false;
        request_redraw();
        return true;

    case gui_mouse_enter:
        hover = true;
        request_redraw();
        return true;

    case gui_mouse_exit:
        hover = false;
        request_redraw();
        return true;

    default:;
    }

    return false;
}

bool widget::handle_command_recursive(command command, std::vector<widget const *> const &reject_list) noexcept
{
    tt_axiom(is_gui_thread());

    auto handled = false;

    auto buffer = pmr::scoped_buffer<256>{};
    for (auto *child : children(buffer.allocator())) {
        if (child) {
            tt_axiom(child->parent == this);
            handled |= child->handle_command_recursive(command, reject_list);
        }
    }

    if (!std::ranges::any_of(reject_list, [this](ttlet &x) {
            return x == this;
        })) {
        handled |= handle_event(command);
    }

    return handled;
}

bool widget::handle_event(mouse_event const &event) noexcept
{
    tt_axiom(is_gui_thread());
    return false;
}

bool widget::handle_event(keyboard_event const &event) noexcept
{
    tt_axiom(is_gui_thread());
    return false;
}

widget const *widget::find_next_widget(
    widget const *current_keyboard_widget,
    keyboard_focus_group group,
    keyboard_focus_direction direction) const noexcept
{
    tt_axiom(is_gui_thread());

    auto found = false;

    if (!current_keyboard_widget && accepts_keyboard_focus(group)) {
        // If there was no current_keyboard_widget, then return this if it accepts focus.
        return this;

    } else if (current_keyboard_widget == this) {
        // If current_keyboard_widget is this, then we need to find the first child widget that accepts focus.
        found = true;
    }

    auto buffer = pmr::scoped_buffer<256>{};
    auto children_copy = make_vector(children(buffer.allocator()));

    if (direction == keyboard_focus_direction::backward) {
        std::reverse(begin(children_copy), end(children_copy));
    }

    for (auto *child : children_copy) {
        if (child) {
            if (found) {
                // Find the first focus accepting widget.
                if (auto tmp = child->find_next_widget({}, group, direction)) {
                    return tmp;
                }

            } else {
                auto tmp = child->find_next_widget(current_keyboard_widget, group, direction);
                if (tmp == current_keyboard_widget) {
                    // The current widget was found, but no next widget available in the child.
                    found = true;

                } else if (tmp) {
                    return tmp;
                }
            }
        }
    }

    if (found) {
        // Either:
        // 1. current_keyboard_widget was {} and this widget, nor its child widgets accept focus.
        // 2. current_keyboard_wigget was this and non of the child widgets accept focus.
        // 3. current_keyboard_widget is a child, and non of the following widgets accept focus.
        return current_keyboard_widget;
    }

    return nullptr;
}

[[nodiscard]] widget const *widget::find_first_widget(keyboard_focus_group group) const noexcept
{
    tt_axiom(is_gui_thread());

    auto buffer = pmr::scoped_buffer<256>{};
    for (auto *child : children(buffer.allocator())) {
        if (child and child->accepts_keyboard_focus(group)) {
            return child;
        }
    }
    return nullptr;
}

[[nodiscard]] widget const *widget::find_last_widget(keyboard_focus_group group) const noexcept
{
    tt_axiom(is_gui_thread());

    auto buffer = pmr::scoped_buffer<256>{};
    widget *found = nullptr;
    for (auto *child : children(buffer.allocator())) {
        if (child and child->accepts_keyboard_focus(group)) {
            found = child;
        }
    }
    return found;
}

[[nodiscard]] bool widget::is_first(keyboard_focus_group group) const noexcept
{
    tt_axiom(is_gui_thread());
    return parent->find_first_widget(group) == this;
}

[[nodiscard]] bool widget::is_last(keyboard_focus_group group) const noexcept
{
    tt_axiom(is_gui_thread());
    return parent->find_last_widget(group) == this;
}

void widget::scroll_to_show(tt::aarectangle rectangle) noexcept
{
    tt_axiom(is_gui_thread());

    if (parent) {
        parent->scroll_to_show(rectangle);
    }
}

/** Get a list of parents of a given widget.
 * The chain includes the given widget.
 */
[[nodiscard]] std::vector<widget const *> widget::parent_chain() const noexcept
{
    tt_axiom(is_gui_thread());

    std::vector<widget const *> chain;

    if (auto w = this) {
        chain.push_back(w);
        while (static_cast<bool>(w = w->parent)) {
            chain.push_back(w);
        }
    }

    return chain;
}

[[nodiscard]] aarectangle widget::make_overlay_rectangle(aarectangle requested_rectangle) const noexcept
{
    tt_axiom(is_gui_thread());

    // Move the request_rectangle to window coordinates.
    ttlet requested_window_rectangle = translate2{_bounding_rectangle - theme().margin} * requested_rectangle;
    ttlet window_bounds = aarectangle{window.screen_rectangle.size()} - theme().margin;
    ttlet response_window_rectangle = fit(window_bounds, requested_window_rectangle);
    return bounding_rectangle(window_to_local() * response_window_rectangle);
}

} // namespace tt
