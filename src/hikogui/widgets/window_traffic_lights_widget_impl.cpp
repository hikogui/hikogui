// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "window_traffic_lights_widget.hpp"
#include "../font/module.hpp"
#include <cmath>
#include <typeinfo>

namespace hi::inline v1 {

window_traffic_lights_widget::window_traffic_lights_widget(widget *parent) noexcept : super(parent) {}

[[nodiscard]] box_constraints window_traffic_lights_widget::update_constraints() noexcept
{
    _layout = {};

    if (theme().operating_system == operating_system::windows) {
        hilet size = extent2i{theme().large_size() * 3, theme().large_size()};
        return {size, size, size};

    } else if (theme().operating_system == operating_system::macos) {
        hilet size = extent2i{DIAMETER * 3 + 2 * MARGIN + 2 * SPACING, DIAMETER + 2 * MARGIN};
        return {size, size, size};

    } else {
        hi_no_default();
    }
}

void window_traffic_lights_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        auto extent = context.size();
        if (extent.height() > narrow_cast<int>(theme().large_size() * 1.2f)) {
            extent = extent2i{extent.width(), theme().large_size()};
        }
        auto y = context.height() - extent.height();

        if (theme().operating_system == operating_system::windows) {
            closeRectangle = aarectanglei{point2i(extent.width() * 2 / 3, y), extent2i{extent.width() * 1 / 3, extent.height()}};

            maximizeRectangle =
                aarectanglei{point2i(extent.width() * 1 / 3, y), extent2i{extent.width() * 1 / 3, extent.height()}};

            minimizeRectangle = aarectanglei{point2i(0, y), extent2i{extent.width() * 1 / 3, extent.height()}};

        } else if (theme().operating_system == operating_system::macos) {
            closeRectangle = aarectanglei{point2i(MARGIN, extent.height() / 2 - RADIUS), extent2i{DIAMETER, DIAMETER}};

            minimizeRectangle =
                aarectanglei{point2i(MARGIN + DIAMETER + SPACING, extent.height() / 2 - RADIUS), extent2i{DIAMETER, DIAMETER}};

            maximizeRectangle = aarectanglei{
                point2i(MARGIN + DIAMETER + SPACING + DIAMETER + SPACING, extent.height() / 2 - RADIUS),
                extent2i{DIAMETER, DIAMETER}};
        } else {
            hi_no_default();
        }

        closeWindowGlyph = find_glyph(hikogui_icon::CloseWindow);
        minimizeWindowGlyph = find_glyph(hikogui_icon::MinimizeWindow);

        if (theme().operating_system == operating_system::windows) {
            maximizeWindowGlyph = find_glyph(hikogui_icon::MaximizeWindowMS);
            restoreWindowGlyph = find_glyph(hikogui_icon::RestoreWindowMS);

        } else if (theme().operating_system == operating_system::macos) {
            maximizeWindowGlyph = find_glyph(hikogui_icon::MaximizeWindowMacOS);
            restoreWindowGlyph = find_glyph(hikogui_icon::RestoreWindowMacOS);
        } else {
            hi_no_default();
        }

        hilet glyph_size = theme().operating_system == operating_system::macos ? 5.0f : theme().icon_size();

        hilet closeWindowGlyphBB = narrow_cast<aarectanglei>(closeWindowGlyph.get_bounding_box() * glyph_size);
        hilet minimizeWindowGlyphBB = narrow_cast<aarectanglei>(minimizeWindowGlyph.get_bounding_box() * glyph_size);
        hilet maximizeWindowGlyphBB = narrow_cast<aarectanglei>(maximizeWindowGlyph.get_bounding_box() * glyph_size);
        hilet restoreWindowGlyphBB = narrow_cast<aarectanglei>(restoreWindowGlyph.get_bounding_box() * glyph_size);

        closeWindowGlyphRectangle = align(closeRectangle, closeWindowGlyphBB, alignment::middle_center());
        minimizeWindowGlyphRectangle = align(minimizeRectangle, minimizeWindowGlyphBB, alignment::middle_center());
        maximizeWindowGlyphRectangle = align(maximizeRectangle, maximizeWindowGlyphBB, alignment::middle_center());
        restoreWindowGlyphRectangle = align(maximizeRectangle, restoreWindowGlyphBB, alignment::middle_center());
    }
}

void window_traffic_lights_widget::drawMacOS(draw_context const& drawContext) noexcept
{
    auto context = drawContext;

    hilet close_circle_color = (not context.active and not *hover) ? color(0.246f, 0.246f, 0.246f) :
        pressedClose                                               ? color(1.0f, 0.242f, 0.212f) :
                                                                     color(1.0f, 0.1f, 0.082f);
    context.draw_box(layout(), closeRectangle, close_circle_color, corner_radii{RADIUS});

    hilet minimize_circle_color = (not context.active and not *hover) ? color(0.246f, 0.246f, 0.246f) :
        pressedMinimize                                               ? color(1.0f, 0.847f, 0.093f) :
                                                                        color(0.784f, 0.521f, 0.021f);
    context.draw_box(layout(), minimizeRectangle, minimize_circle_color, corner_radii{RADIUS});

    hilet maximize_circle_color = (not context.active and not *hover) ? color(0.246f, 0.246f, 0.246f) :
        pressedMaximize                                               ? color(0.223f, 0.863f, 0.1f) :
                                                                        color(0.082f, 0.533f, 0.024f);

    context.draw_box(layout(), maximizeRectangle, maximize_circle_color, corner_radii{RADIUS});

    if (*hover) {
        context.draw_glyph(
            layout(),
            translate_z(0.1f) * narrow_cast<aarectangle>(closeWindowGlyphRectangle),
            closeWindowGlyph,
            color{0.319f, 0.0f, 0.0f});
        context.draw_glyph(
            layout(),
            translate_z(0.1f) * narrow_cast<aarectangle>(minimizeWindowGlyphRectangle),
            minimizeWindowGlyph,
            color{0.212f, 0.1f, 0.0f});

        if (layout().window_size_state == gui_window_size::maximized) {
            context.draw_glyph(
                layout(),
                translate_z(0.1f) * narrow_cast<aarectangle>(restoreWindowGlyphRectangle),
                restoreWindowGlyph,
                color{0.0f, 0.133f, 0.0f});
        } else {
            context.draw_glyph(
                layout(),
                translate_z(0.1f) * narrow_cast<aarectangle>(maximizeWindowGlyphRectangle),
                maximizeWindowGlyph,
                color{0.0f, 0.133f, 0.0f});
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
        context.draw_box(layout(), closeRectangle, theme().color(semantic_color::fill, semantic_layer));
    }

    if (pressedMinimize) {
        context.draw_box(
            layout(), minimizeRectangle, theme().color(semantic_color::fill, semantic_layer + 2));
    } else if (hoverMinimize) {
        context.draw_box(
            layout(), minimizeRectangle, theme().color(semantic_color::fill, semantic_layer + 1));
    } else {
        context.draw_box(
            layout(), minimizeRectangle, theme().color(semantic_color::fill, semantic_layer));
    }

    if (pressedMaximize) {
        context.draw_box(
            layout(), maximizeRectangle, theme().color(semantic_color::fill, semantic_layer + 2));
    } else if (hoverMaximize) {
        context.draw_box(
            layout(), maximizeRectangle, theme().color(semantic_color::fill, semantic_layer + 1));
    } else {
        context.draw_box(
            layout(), maximizeRectangle, theme().color(semantic_color::fill, semantic_layer));
    }

    hilet glyph_color = context.active ? label_color() : foreground_color();

    context.draw_glyph(
        layout(), translate_z(0.1f) * narrow_cast<aarectangle>(closeWindowGlyphRectangle), closeWindowGlyph, glyph_color);
    context.draw_glyph(
        layout(), translate_z(0.1f) * narrow_cast<aarectangle>(minimizeWindowGlyphRectangle), minimizeWindowGlyph, glyph_color);
    if (layout().window_size_state == gui_window_size::maximized) {
        context.draw_glyph(
            layout(), translate_z(0.1f) * narrow_cast<aarectangle>(restoreWindowGlyphRectangle), restoreWindowGlyph, glyph_color);
    } else {
        context.draw_glyph(
            layout(),
            translate_z(0.1f) * narrow_cast<aarectangle>(maximizeWindowGlyphRectangle),
            maximizeWindowGlyph,
            glyph_color);
    }
}

void window_traffic_lights_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
        if (theme().operating_system == operating_system::macos) {
            drawMacOS(context);

        } else if (theme().operating_system == operating_system::windows) {
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

hitbox window_traffic_lights_widget::hitbox_test(point2i position) const noexcept
{
    hi_axiom(loop::main().on_thread());

    if (*mode >= widget_mode::partial and layout().contains(position) and
        (closeRectangle.contains(position) or minimizeRectangle.contains(position) or maximizeRectangle.contains(position))) {
        return hitbox{id, _layout.elevation, hitbox_type::button};
    } else {
        return {};
    }
}

} // namespace hi::inline v1
