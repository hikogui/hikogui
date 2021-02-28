// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "window_traffic_lights_widget.hpp"
#include "../GUI/utils.hpp"
#include "../text/ttauri_icon.hpp"
#include "../utils.hpp"
#include <cmath>
#include <typeinfo>

namespace tt {

window_traffic_lights_widget::window_traffic_lights_widget(
    gui_window &window,
    std::shared_ptr<abstract_container_widget> parent) noexcept :
    super(window, parent)
{
    // Toolbar buttons hug the toolbar and neighbor widgets.
    _margin = 0.0f;
}

[[nodiscard]] bool
window_traffic_lights_widget::update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    if (super::update_constraints(display_time_point, need_reconstrain)) {
        if constexpr (theme::global->operatingSystem == OperatingSystem::Windows) {
            ttlet width = theme::global->toolbarDecorationButtonWidth * 3.0f;
            ttlet height = theme::global->toolbarHeight;
            _preferred_size = {extent2{width, height}, extent2{width, std::numeric_limits<float>::infinity()}};

        } else if constexpr (theme::global->operatingSystem == OperatingSystem::MacOS) {
            ttlet width = DIAMETER * 3.0f + 2.0f * MARGIN + 2.0f * SPACING;
            ttlet height = DIAMETER + 2.0f * MARGIN;
            _preferred_size = {extent2{width, height}, extent2{width, std::numeric_limits<float>::infinity()}};

        } else {
            tt_no_default();
        }
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] void
window_traffic_lights_widget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    need_layout |= std::exchange(_request_relayout, false);
    if (need_layout) {
        auto extent = rectangle().extent();
        if (extent.height() > theme::global->toolbarHeight * 1.2f) {
            extent = extent2{extent.width(), theme::global->toolbarHeight};
        }
        auto y = rectangle().height() - extent.height();

        if constexpr (theme::global->operatingSystem == OperatingSystem::Windows) {
            closeRectangle =
                aarect{f32x4::point(extent.width() * 2.0f / 3.0f, y), f32x4{extent.width() * 1.0f / 3.0f, extent.height()}};

            maximizeRectangle =
                aarect{f32x4::point(extent.width() * 1.0f / 3.0f, y), f32x4{extent.width() * 1.0f / 3.0f, extent.height()}};

            minimizeRectangle = aarect{f32x4::point(0.0f, y), f32x4{extent.width() * 1.0f / 3.0f, extent.height()}};

        } else if constexpr (theme::global->operatingSystem == OperatingSystem::MacOS) {
            closeRectangle = aarect{f32x4::point(MARGIN, extent.height() / 2.0f - RADIUS), {DIAMETER, DIAMETER}};

            minimizeRectangle =
                aarect{f32x4::point(MARGIN + DIAMETER + SPACING, extent.height() / 2.0f - RADIUS), {DIAMETER, DIAMETER}};

            maximizeRectangle = aarect{
                f32x4::point(MARGIN + DIAMETER + SPACING + DIAMETER + SPACING, extent.height() / 2.0f - RADIUS),
                {DIAMETER, DIAMETER}};
        } else {
            tt_no_default();
        }

        closeWindowGlyph = to_font_glyph_ids(ttauri_icon::CloseWindow);
        minimizeWindowGlyph = to_font_glyph_ids(ttauri_icon::MinimizeWindow);

        if constexpr (theme::global->operatingSystem == OperatingSystem::Windows) {
            maximizeWindowGlyph = to_font_glyph_ids(ttauri_icon::MaximizeWindowMS);
            restoreWindowGlyph = to_font_glyph_ids(ttauri_icon::RestoreWindowMS);

        } else if constexpr (theme::global->operatingSystem == OperatingSystem::MacOS) {
            maximizeWindowGlyph = to_font_glyph_ids(ttauri_icon::MaximizeWindowMacOS);
            restoreWindowGlyph = to_font_glyph_ids(ttauri_icon::RestoreWindowMacOS);
        } else {
            tt_no_default();
        }

        ttlet closeWindowGlyphBB = pipeline_SDF::device_shared::getBoundingBox(closeWindowGlyph);
        ttlet minimizeWindowGlyphBB = pipeline_SDF::device_shared::getBoundingBox(minimizeWindowGlyph);
        ttlet maximizeWindowGlyphBB = pipeline_SDF::device_shared::getBoundingBox(maximizeWindowGlyph);
        ttlet restoreWindowGlyphBB = pipeline_SDF::device_shared::getBoundingBox(restoreWindowGlyph);

        ttlet glyph_size = theme::global->operatingSystem == OperatingSystem::MacOS ? 5.0f : theme::global->small_icon_size;

        closeWindowGlyphRectangle = align(closeRectangle, scale(closeWindowGlyphBB, glyph_size), alignment::middle_center);
        minimizeWindowGlyphRectangle =
            align(minimizeRectangle, scale(minimizeWindowGlyphBB, glyph_size), alignment::middle_center);
        maximizeWindowGlyphRectangle =
            align(maximizeRectangle, scale(maximizeWindowGlyphBB, glyph_size), alignment::middle_center);
        restoreWindowGlyphRectangle = align(maximizeRectangle, scale(restoreWindowGlyphBB, glyph_size), alignment::middle_center);
    }
    super::update_layout(display_time_point, need_layout);
}

void window_traffic_lights_widget::drawMacOS(
    draw_context const &drawContext,
    hires_utc_clock::time_point displayTimePoint) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    auto context = drawContext;

    ttlet close_circle_color = (!window.active && !_hover) ? color(0.246f, 0.246f, 0.246f) :
        pressedClose                                       ? color(1.0f, 0.242f, 0.212f) :
                                                             color(1.0f, 0.1f, 0.082f);
    context.draw_box(closeRectangle, close_circle_color, corner_shapes{RADIUS});

    ttlet minimize_circle_color = (!window.active && !_hover) ? color(0.246f, 0.246f, 0.246f) :
        pressedMinimize                                       ? color(1.0f, 0.847f, 0.093f) :
                                                                color(0.784f, 0.521f, 0.021f);
    context.draw_box(minimizeRectangle, minimize_circle_color, corner_shapes{RADIUS});

    ttlet maximize_circle_color = (!window.active && !_hover) ? color(0.246f, 0.246f, 0.246f) :
        pressedMaximize                                       ? color(0.223f, 0.863f, 0.1f) :
                                                                color(0.082f, 0.533f, 0.024f);

    context.draw_box(maximizeRectangle, maximize_circle_color, corner_shapes{RADIUS});

    if (_hover) {
        context.draw_glyph(closeWindowGlyph, translate_z(0.1f) * closeWindowGlyphRectangle, color{0.319f, 0.0f, 0.0f});
        context.draw_glyph(minimizeWindowGlyph, translate_z(0.1f) * minimizeWindowGlyphRectangle, color{0.212f, 0.1f, 0.0f});

        if (window.size_state == gui_window_size::maximized) {
            context.draw_glyph(restoreWindowGlyph, translate_z(0.1f) * restoreWindowGlyphRectangle, color{0.0f, 0.133f, 0.0f});
        } else {
            context.draw_glyph(maximizeWindowGlyph, translate_z(0.1f) * maximizeWindowGlyphRectangle, color{0.0f, 0.133f, 0.0f});
        }
    }
}

void window_traffic_lights_widget::drawWindows(
    draw_context const &drawContext,
    hires_utc_clock::time_point displayTimePoint) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    auto context = drawContext;

    if (pressedClose) {
        context.draw_filled_quad(closeRectangle, color{1.0f, 0.0f, 0.0f});
    } else if (hoverClose) {
        context.draw_filled_quad(closeRectangle, color{0.5f, 0.0f, 0.0f});
    } else {
        context.draw_filled_quad(closeRectangle, theme::global->fillColor(_semantic_layer));
    }

    if (pressedMinimize) {
        context.draw_filled_quad(minimizeRectangle, theme::global->fillColor(_semantic_layer + 2));
    } else if (hoverMinimize) {
        context.draw_filled_quad(minimizeRectangle, theme::global->fillColor(_semantic_layer + 1));
    } else {
        context.draw_filled_quad(minimizeRectangle, theme::global->fillColor(_semantic_layer));
    }

    if (pressedMaximize) {
        context.draw_filled_quad(maximizeRectangle, theme::global->fillColor(_semantic_layer + 2));
    } else if (hoverMaximize) {
        context.draw_filled_quad(maximizeRectangle, theme::global->fillColor(_semantic_layer + 1));
    } else {
        context.draw_filled_quad(maximizeRectangle, theme::global->fillColor(_semantic_layer));
    }

    ttlet glyph_color = window.active ? label_color() : foreground_color();

    context.draw_glyph(closeWindowGlyph, translate_z(0.1f) * closeWindowGlyphRectangle, glyph_color);
    context.draw_glyph(minimizeWindowGlyph, translate_z(0.1f) * minimizeWindowGlyphRectangle, glyph_color);
    if (window.size_state == gui_window_size::maximized) {
        context.draw_glyph(restoreWindowGlyph, translate_z(0.1f) * restoreWindowGlyphRectangle, glyph_color);
    } else {
        context.draw_glyph(maximizeWindowGlyph, translate_z(0.1f) * maximizeWindowGlyphRectangle, glyph_color);
    }
}

void window_traffic_lights_widget::draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_axiom(gui_system_mutex.recurse_lock_count());

    if (overlaps(context, _clipping_rectangle)) {
        if constexpr (theme::global->operatingSystem == OperatingSystem::MacOS) {
            drawMacOS(context, display_time_point);

        } else if constexpr (theme::global->operatingSystem == OperatingSystem::Windows) {
            drawWindows(context, display_time_point);

        } else {
            tt_no_default();
        }
    }

    super::draw(std::move(context), display_time_point);
}

bool window_traffic_lights_widget::handle_event(mouse_event const &event) noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);
    auto handled = super::handle_event(event);

    // Check the hover states of each button.
    auto stateHasChanged = false;
    stateHasChanged |= compare_then_assign(hoverClose, closeRectangle.contains(event.position));
    stateHasChanged |= compare_then_assign(hoverMinimize, minimizeRectangle.contains(event.position));
    stateHasChanged |= compare_then_assign(hoverMaximize, maximizeRectangle.contains(event.position));
    if (stateHasChanged) {
        window.request_redraw(aarect{_local_to_window * _clipping_rectangle});
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

            window.request_redraw(aarect{_local_to_window * _clipping_rectangle});
            pressedClose = false;
            pressedMinimize = false;
            pressedMaximize = false;
            break;

        case ButtonDown:
            window.request_redraw(aarect{_local_to_window * _clipping_rectangle});
            pressedClose = hoverClose;
            pressedMinimize = hoverMinimize;
            pressedMaximize = hoverMaximize;
            break;
        }
    }

    return handled;
}

hit_box window_traffic_lights_widget::hitbox_test(point2 position) const noexcept
{
    ttlet lock = std::scoped_lock(gui_system_mutex);

    if (rectangle().contains(position)) {
        if (closeRectangle.contains(position) || minimizeRectangle.contains(position) || maximizeRectangle.contains(position)) {
            return hit_box{weak_from_this(), _draw_layer, hit_box::Type::Button};
        } else {
            return hit_box{};
        }
    } else {
        return {};
    }
}

} // namespace tt
