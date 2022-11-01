// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "window_traffic_lights_widget.hpp"
#include "../text/font_book.hpp"
#include <cmath>
#include <typeinfo>

namespace hi::inline v1 {

window_traffic_lights_widget::window_traffic_lights_widget(widget *parent) noexcept : super(parent) {}

widget_constraints const& window_traffic_lights_widget::set_constraints(set_constraints_context const &context) noexcept
{
    _layout = {};

    if (context.theme->operating_system == operating_system::windows) {
        hilet size = extent2{context.theme->large_size * 3.0f, context.theme->large_size};
        return _constraints = {size, size, size};

    } else if (context.theme->operating_system == operating_system::macos) {
        hilet size = extent2{DIAMETER * 3.0f + 2.0f * MARGIN + 2.0f * SPACING, DIAMETER + 2.0f * MARGIN};
        return _constraints = {size, size, size};

    } else {
        hi_no_default();
    }
}

void window_traffic_lights_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        auto extent = context.size;
        if (extent.height() > context.theme->large_size * 1.2f) {
            extent = extent2{extent.width(), context.theme->large_size};
        }
        auto y = context.height() - extent.height();

        if (context.theme->operating_system == operating_system::windows) {
            closeRectangle =
                aarectangle{point2(extent.width() * 2.0f / 3.0f, y), extent2{extent.width() * 1.0f / 3.0f, extent.height()}};

            maximizeRectangle =
                aarectangle{point2(extent.width() * 1.0f / 3.0f, y), extent2{extent.width() * 1.0f / 3.0f, extent.height()}};

            minimizeRectangle = aarectangle{point2(0.0f, y), extent2{extent.width() * 1.0f / 3.0f, extent.height()}};

        } else if (context.theme->operating_system == operating_system::macos) {
            closeRectangle = aarectangle{point2(MARGIN, extent.height() / 2.0f - RADIUS), extent2{DIAMETER, DIAMETER}};

            minimizeRectangle =
                aarectangle{point2(MARGIN + DIAMETER + SPACING, extent.height() / 2.0f - RADIUS), extent2{DIAMETER, DIAMETER}};

            maximizeRectangle = aarectangle{
                point2(MARGIN + DIAMETER + SPACING + DIAMETER + SPACING, extent.height() / 2.0f - RADIUS),
                extent2{DIAMETER, DIAMETER}};
        } else {
            hi_no_default();
        }

        closeWindowGlyph = context.font_book->find_glyph(hikogui_icon::CloseWindow);
        minimizeWindowGlyph = context.font_book->find_glyph(hikogui_icon::MinimizeWindow);

        if (context.theme->operating_system == operating_system::windows) {
            maximizeWindowGlyph = context.font_book->find_glyph(hikogui_icon::MaximizeWindowMS);
            restoreWindowGlyph = context.font_book->find_glyph(hikogui_icon::RestoreWindowMS);

        } else if (context.theme->operating_system == operating_system::macos) {
            maximizeWindowGlyph = context.font_book->find_glyph(hikogui_icon::MaximizeWindowMacOS);
            restoreWindowGlyph = context.font_book->find_glyph(hikogui_icon::RestoreWindowMacOS);
        } else {
            hi_no_default();
        }

        hilet closeWindowGlyphBB = closeWindowGlyph.get_bounding_box();
        hilet minimizeWindowGlyphBB = minimizeWindowGlyph.get_bounding_box();
        hilet maximizeWindowGlyphBB = maximizeWindowGlyph.get_bounding_box();
        hilet restoreWindowGlyphBB = restoreWindowGlyph.get_bounding_box();

        hilet glyph_size = context.theme->operating_system == operating_system::macos ? 5.0f : context.theme->icon_size;

        closeWindowGlyphRectangle = align(closeRectangle, closeWindowGlyphBB * glyph_size, alignment::middle_center());
        minimizeWindowGlyphRectangle = align(minimizeRectangle, minimizeWindowGlyphBB * glyph_size, alignment::middle_center());
        maximizeWindowGlyphRectangle = align(maximizeRectangle, maximizeWindowGlyphBB * glyph_size, alignment::middle_center());
        restoreWindowGlyphRectangle = align(maximizeRectangle, restoreWindowGlyphBB * glyph_size, alignment::middle_center());
    }
}

void window_traffic_lights_widget::drawMacOS(draw_context const& drawContext) noexcept
{
    auto context = drawContext;

    hilet close_circle_color = (not context.active and not *hover) ? color(0.246f, 0.246f, 0.246f) :
        pressedClose                                              ? color(1.0f, 0.242f, 0.212f) :
                                                                    color(1.0f, 0.1f, 0.082f);
    context.draw_box(layout(), closeRectangle, close_circle_color, corner_radii{RADIUS});

    hilet minimize_circle_color = (not context.active and not *hover) ? color(0.246f, 0.246f, 0.246f) :
        pressedMinimize                                              ? color(1.0f, 0.847f, 0.093f) :
                                                                       color(0.784f, 0.521f, 0.021f);
    context.draw_box(layout(), minimizeRectangle, minimize_circle_color, corner_radii{RADIUS});

    hilet maximize_circle_color = (not context.active and not *hover) ? color(0.246f, 0.246f, 0.246f) :
        pressedMaximize                                              ? color(0.223f, 0.863f, 0.1f) :
                                                                       color(0.082f, 0.533f, 0.024f);

    context.draw_box(layout(), maximizeRectangle, maximize_circle_color, corner_radii{RADIUS});

    if (*hover) {
        context.draw_glyph(layout(), translate_z(0.1f) * closeWindowGlyphRectangle, color{0.319f, 0.0f, 0.0f}, closeWindowGlyph);
        context.draw_glyph(
            layout(), translate_z(0.1f) * minimizeWindowGlyphRectangle, color{0.212f, 0.1f, 0.0f}, minimizeWindowGlyph);

        if (layout().window_size_state == gui_window_size::maximized) {
            context.draw_glyph(
                layout(), translate_z(0.1f) * restoreWindowGlyphRectangle, color{0.0f, 0.133f, 0.0f}, restoreWindowGlyph);
        } else {
            context.draw_glyph(
                layout(), translate_z(0.1f) * maximizeWindowGlyphRectangle, color{0.0f, 0.133f, 0.0f}, maximizeWindowGlyph);
        }
    }
}

void window_traffic_lights_widget::drawWindows(draw_context const& drawContext) noexcept
{
    auto context = drawContext;

    if (pressedClose) {
        context.draw_box(layout(), closeRectangle, color{1.0f, 0.0f, 0.0f});
    } else if (hoverClose) {
        context.draw_box(layout(), closeRectangle, color{0.5f, 0.0f, 0.0f});
    } else {
        context.draw_box(layout(), closeRectangle, layout().theme->color(semantic_color::fill, semantic_layer));
    }

    if (pressedMinimize) {
        context.draw_box(layout(), minimizeRectangle, layout().theme->color(semantic_color::fill, semantic_layer + 2));
    } else if (hoverMinimize) {
        context.draw_box(layout(), minimizeRectangle, layout().theme->color(semantic_color::fill, semantic_layer + 1));
    } else {
        context.draw_box(layout(), minimizeRectangle, layout().theme->color(semantic_color::fill, semantic_layer));
    }

    if (pressedMaximize) {
        context.draw_box(layout(), maximizeRectangle, layout().theme->color(semantic_color::fill, semantic_layer + 2));
    } else if (hoverMaximize) {
        context.draw_box(layout(), maximizeRectangle, layout().theme->color(semantic_color::fill, semantic_layer + 1));
    } else {
        context.draw_box(layout(), maximizeRectangle, layout().theme->color(semantic_color::fill, semantic_layer));
    }

    hilet glyph_color = context.active ? label_color() : foreground_color();

    context.draw_glyph(layout(), translate_z(0.1f) * closeWindowGlyphRectangle, glyph_color, closeWindowGlyph);
    context.draw_glyph(layout(), translate_z(0.1f) * minimizeWindowGlyphRectangle, glyph_color, minimizeWindowGlyph);
    if (layout().window_size_state == gui_window_size::maximized) {
        context.draw_glyph(layout(), translate_z(0.1f) * restoreWindowGlyphRectangle, glyph_color, restoreWindowGlyph);
    } else {
        context.draw_glyph(layout(), translate_z(0.1f) * maximizeWindowGlyphRectangle, glyph_color, maximizeWindowGlyph);
    }
}

void window_traffic_lights_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
        if (layout().theme->operating_system == operating_system::macos) {
            drawMacOS(context);

        } else if (layout().theme->operating_system == operating_system::windows) {
            drawWindows(context);

        } else {
            hi_no_default();
        }
    }
}

bool window_traffic_lights_widget::handle_event(gui_event const& event) noexcept
{
    switch (event.type()) {
    case gui_event_type::mouse_move:
    case gui_event_type::mouse_drag:
        {
            // Check the hover states of each button.
            auto state_has_changed = false;
            state_has_changed |= compare_store(hoverClose, closeRectangle.contains(event.mouse().position));
            state_has_changed |= compare_store(hoverMinimize, minimizeRectangle.contains(event.mouse().position));
            state_has_changed |= compare_store(hoverMaximize, maximizeRectangle.contains(event.mouse().position));
            if (state_has_changed) {
                request_redraw();
            }
        }
        break;

    case gui_event_type::mouse_exit:
        hoverClose = false;
        hoverMinimize = false;
        hoverMaximize = false;
        request_redraw();
        return super::handle_event(event);

    case gui_event_type::mouse_down:
        if (event.mouse().cause.left_button) {
            if (closeRectangle.contains(event.mouse().position)) {
                pressedClose = true;

            } else if (minimizeRectangle.contains(event.mouse().position)) {
                pressedMinimize = true;

            } else if (maximizeRectangle.contains(event.mouse().position)) {
                pressedMaximize = true;
            }
            request_redraw();
            return true;
        }
        break;

    case gui_event_type::mouse_up:
        if (event.mouse().cause.left_button) {
            pressedClose = false;
            pressedMinimize = false;
            pressedMaximize = false;
            request_redraw();

            if (closeRectangle.contains(event.mouse().position)) {
                return process_event({gui_event_type::window_close});

            } else if (minimizeRectangle.contains(event.mouse().position)) {
                return process_event({gui_event_type::window_minimize});

            } else if (maximizeRectangle.contains(event.mouse().position)) {
                switch (layout().window_size_state) {
                case gui_window_size::normal:
                    return process_event({gui_event_type::window_maximize});

                case gui_window_size::maximized:
                    return process_event({gui_event_type::window_normalize});

                default:
                    hi_no_default();
                }
            }
            return true;
        }
        break;

    default:;
    }
    return super::handle_event(event);
}

hitbox window_traffic_lights_widget::hitbox_test(point3 position) const noexcept
{
    hi_axiom(loop::main().on_thread());

    if (*mode >= widget_mode::partial and layout().contains(position) and
        (closeRectangle.contains(position) or minimizeRectangle.contains(position) or maximizeRectangle.contains(position))) {
        return hitbox{this, position, hitbox_type::button};
    } else {
        return {};
    }
}

} // namespace hi::inline v1
