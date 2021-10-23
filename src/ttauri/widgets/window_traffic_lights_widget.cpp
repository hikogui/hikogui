// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "window_traffic_lights_widget.hpp"
#include "../GUI/gui_window.hpp"
#include "../GFX/pipeline_SDF_device_shared.hpp"
#include "../text/font_book.hpp"
#include <cmath>
#include <typeinfo>

namespace tt {

window_traffic_lights_widget::window_traffic_lights_widget(gui_window &window, widget *parent) noexcept : super(window, parent) {}

[[nodiscard]] float window_traffic_lights_widget::margin() const noexcept
{
    return 0.0f;
}

widget_constraints const &window_traffic_lights_widget::set_constraints() noexcept
{
    tt_axiom(is_gui_thread());

    _layout = {};

    if (theme().operating_system == operating_system::windows) {
        ttlet width = theme().toolbar_decoration_button_width * 3.0f;
        ttlet height = theme().toolbar_height;
        _constraints.min = _constraints.pref = _constraints.max = {width, height};

    } else if (theme().operating_system == operating_system::macos) {
        ttlet width = DIAMETER * 3.0f + 2.0f * MARGIN + 2.0f * SPACING;
        ttlet height = DIAMETER + 2.0f * MARGIN;
        _constraints.min = _constraints.pref = _constraints.max = {width, height};

    } else {
        tt_no_default();
    }
    tt_axiom(_constraints.min <= _constraints.pref && _constraints.pref <= _constraints.max);
    return _constraints;
}

void window_traffic_lights_widget::set_layout(widget_layout const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and _layout.store(context) >= layout_update::transform) {
        auto extent = layout().size;
        if (extent.height() > theme().toolbar_height * 1.2f) {
            extent = extent2{extent.width(), theme().toolbar_height};
        }
        auto y = layout().height() - extent.height();

        if (theme().operating_system == operating_system::windows) {
            closeRectangle =
                aarectangle{point2(extent.width() * 2.0f / 3.0f, y), extent2{extent.width() * 1.0f / 3.0f, extent.height()}};

            maximizeRectangle =
                aarectangle{point2(extent.width() * 1.0f / 3.0f, y), extent2{extent.width() * 1.0f / 3.0f, extent.height()}};

            minimizeRectangle = aarectangle{point2(0.0f, y), extent2{extent.width() * 1.0f / 3.0f, extent.height()}};

        } else if (theme().operating_system == operating_system::macos) {
            closeRectangle = aarectangle{point2(MARGIN, extent.height() / 2.0f - RADIUS), extent2{DIAMETER, DIAMETER}};

            minimizeRectangle =
                aarectangle{point2(MARGIN + DIAMETER + SPACING, extent.height() / 2.0f - RADIUS), extent2{DIAMETER, DIAMETER}};

            maximizeRectangle = aarectangle{
                point2(MARGIN + DIAMETER + SPACING + DIAMETER + SPACING, extent.height() / 2.0f - RADIUS),
                extent2{DIAMETER, DIAMETER}};
        } else {
            tt_no_default();
        }

        closeWindowGlyph = font_book().find_glyph(ttauri_icon::CloseWindow);
        minimizeWindowGlyph = font_book().find_glyph(ttauri_icon::MinimizeWindow);

        if (theme().operating_system == operating_system::windows) {
            maximizeWindowGlyph = font_book().find_glyph(ttauri_icon::MaximizeWindowMS);
            restoreWindowGlyph = font_book().find_glyph(ttauri_icon::RestoreWindowMS);

        } else if (theme().operating_system == operating_system::macos) {
            maximizeWindowGlyph = font_book().find_glyph(ttauri_icon::MaximizeWindowMacOS);
            restoreWindowGlyph = font_book().find_glyph(ttauri_icon::RestoreWindowMacOS);
        } else {
            tt_no_default();
        }

        ttlet closeWindowGlyphBB = closeWindowGlyph.get_bounding_box();
        ttlet minimizeWindowGlyphBB = minimizeWindowGlyph.get_bounding_box();
        ttlet maximizeWindowGlyphBB = maximizeWindowGlyph.get_bounding_box();
        ttlet restoreWindowGlyphBB = restoreWindowGlyph.get_bounding_box();

        ttlet glyph_size = theme().operating_system == operating_system::macos ? 5.0f : theme().icon_size;

        closeWindowGlyphRectangle = align(closeRectangle, closeWindowGlyphBB * glyph_size, alignment::middle_center);
        minimizeWindowGlyphRectangle = align(minimizeRectangle, minimizeWindowGlyphBB * glyph_size, alignment::middle_center);
        maximizeWindowGlyphRectangle = align(maximizeRectangle, maximizeWindowGlyphBB * glyph_size, alignment::middle_center);
        restoreWindowGlyphRectangle = align(maximizeRectangle, restoreWindowGlyphBB * glyph_size, alignment::middle_center);
    }
}

void window_traffic_lights_widget::drawMacOS(draw_context const &drawContext) noexcept
{
    tt_axiom(is_gui_thread());

    auto context = drawContext;

    ttlet close_circle_color = (!window.active && !hover) ? color(0.246f, 0.246f, 0.246f) :
        pressedClose                                      ? color(1.0f, 0.242f, 0.212f) :
                                                            color(1.0f, 0.1f, 0.082f);
    context.draw_box(layout(), closeRectangle, close_circle_color, corner_shapes{RADIUS});

    ttlet minimize_circle_color = (!window.active && !hover) ? color(0.246f, 0.246f, 0.246f) :
        pressedMinimize                                      ? color(1.0f, 0.847f, 0.093f) :
                                                               color(0.784f, 0.521f, 0.021f);
    context.draw_box(layout(), minimizeRectangle, minimize_circle_color, corner_shapes{RADIUS});

    ttlet maximize_circle_color = (!window.active && !hover) ? color(0.246f, 0.246f, 0.246f) :
        pressedMaximize                                      ? color(0.223f, 0.863f, 0.1f) :
                                                               color(0.082f, 0.533f, 0.024f);

    context.draw_box(layout(), maximizeRectangle, maximize_circle_color, corner_shapes{RADIUS});

    if (hover) {
        context.draw_glyph(
            layout(), closeWindowGlyph, translate_z(0.1f) * closeWindowGlyphRectangle, color{0.319f, 0.0f, 0.0f});
        context.draw_glyph(
            layout(),
            minimizeWindowGlyph,
            translate_z(0.1f) * minimizeWindowGlyphRectangle,
            color{0.212f, 0.1f, 0.0f});

        if (window.size_state == gui_window_size::maximized) {
            context.draw_glyph(
                layout(),
                restoreWindowGlyph,
                translate_z(0.1f) * restoreWindowGlyphRectangle,
                color{0.0f, 0.133f, 0.0f});
        } else {
            context.draw_glyph(
                layout(),
                maximizeWindowGlyph,
                translate_z(0.1f) * maximizeWindowGlyphRectangle,
                color{0.0f, 0.133f, 0.0f});
        }
    }
}

void window_traffic_lights_widget::drawWindows(draw_context const &drawContext) noexcept
{
    tt_axiom(is_gui_thread());

    auto context = drawContext;

    if (pressedClose) {
        context.draw_box(layout(), closeRectangle, color{1.0f, 0.0f, 0.0f});
    } else if (hoverClose) {
        context.draw_box(layout(), closeRectangle, color{0.5f, 0.0f, 0.0f});
    } else {
        context.draw_box(layout(), closeRectangle, theme().color(theme_color::fill, semantic_layer));
    }

    if (pressedMinimize) {
        context.draw_box(layout(), minimizeRectangle, theme().color(theme_color::fill, semantic_layer + 2));
    } else if (hoverMinimize) {
        context.draw_box(layout(), minimizeRectangle, theme().color(theme_color::fill, semantic_layer + 1));
    } else {
        context.draw_box(layout(), minimizeRectangle, theme().color(theme_color::fill, semantic_layer));
    }

    if (pressedMaximize) {
        context.draw_box(layout(), maximizeRectangle, theme().color(theme_color::fill, semantic_layer + 2));
    } else if (hoverMaximize) {
        context.draw_box(layout(), maximizeRectangle, theme().color(theme_color::fill, semantic_layer + 1));
    } else {
        context.draw_box(layout(), maximizeRectangle, theme().color(theme_color::fill, semantic_layer));
    }

    ttlet glyph_color = window.active ? label_color() : foreground_color();

    context.draw_glyph(layout(), closeWindowGlyph, translate_z(0.1f) * closeWindowGlyphRectangle, glyph_color);
    context.draw_glyph(layout(), minimizeWindowGlyph, translate_z(0.1f) * minimizeWindowGlyphRectangle, glyph_color);
    if (window.size_state == gui_window_size::maximized) {
        context.draw_glyph(
            layout(), restoreWindowGlyph, translate_z(0.1f) * restoreWindowGlyphRectangle, glyph_color);
    } else {
        context.draw_glyph(
            layout(), maximizeWindowGlyph, translate_z(0.1f) * maximizeWindowGlyphRectangle, glyph_color);
    }
}

void window_traffic_lights_widget::draw(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and overlaps(context, layout())) {
        if (theme().operating_system == operating_system::macos) {
            drawMacOS(context);

        } else if (theme().operating_system == operating_system::windows) {
            drawWindows(context);

        } else {
            tt_no_default();
        }
    }
}

bool window_traffic_lights_widget::handle_event(mouse_event const &event) noexcept
{
    tt_axiom(is_gui_thread());
    auto handled = super::handle_event(event);

    // Check the hover states of each button.
    auto stateHasChanged = false;
    stateHasChanged |= compare_then_assign(hoverClose, closeRectangle.contains(event.position));
    stateHasChanged |= compare_then_assign(hoverMinimize, minimizeRectangle.contains(event.position));
    stateHasChanged |= compare_then_assign(hoverMaximize, maximizeRectangle.contains(event.position));
    if (stateHasChanged) {
        request_redraw();
    }

    if (event.cause.leftButton) {
        handled = true;

        switch (event.type) {
            using enum mouse_event::Type;
        case ButtonUp:
            if (pressedClose && hoverClose) {
                window.close_window();
            }

            if (pressedMinimize && hoverMinimize) {
                window.minimize_window();
            }

            if (pressedMaximize && hoverMaximize) {
                switch (window.size_state) {
                case gui_window_size::normal: window.maximize_window(); break;
                case gui_window_size::maximized: window.normalize_window(); break;
                default: tt_no_default();
                }
            }

            request_redraw();
            pressedClose = false;
            pressedMinimize = false;
            pressedMaximize = false;
            break;

        case ButtonDown:
            request_redraw();
            pressedClose = hoverClose;
            pressedMinimize = hoverMinimize;
            pressedMaximize = hoverMaximize;
            break;
        }
    }

    return handled;
}

hitbox window_traffic_lights_widget::hitbox_test(point3 position) const noexcept
{
    tt_axiom(is_gui_thread());

    if (layout().hit_rectangle.contains(position)) {
        if (closeRectangle.contains(position) || minimizeRectangle.contains(position) || maximizeRectangle.contains(position)) {
            return hitbox{this, position, hitbox::Type::Button};
        } else {
            return hitbox{};
        }
    } else {
        return {};
    }
}

} // namespace tt
