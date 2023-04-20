// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/window_traffic_lights_widget.hpp Defines window_traffic_lights_widget.
 * @ingroup widgets
 */

#pragma once

#include "../GUI/module.hpp"
#include "../font/module.hpp"
#include <memory>
#include <string>
#include <array>

namespace hi { inline namespace v1 {

/** Window control button widget.
 * This widget will display a set of buttons to control the
 * window. Most common these buttons are the minimize, maximize/restore and close buttons.
 * On MacOS these are red, green and yellow which gives them the traffic lights name.
 *
 * @ingroup widgets
 */
template<fixed_string Name = "">
class window_traffic_lights_widget final : public widget {
public:
    using super = widget;
    constexpr static auto prefix = Name / "traffic-lights";

    window_traffic_lights_widget(widget *parent) noexcept : super(parent) {}

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        if (operating_system::current == operating_system::windows) {
            hilet theme_size = theme<prefix / "windows.size", int>{}(this);
            hilet size = extent2i{theme_size * 3, theme_size};
            return {size, size, size};

        } else if (operating_system::current == operating_system::macos) {
            hilet theme_size = theme<prefix / "macos.size", int>{}(this);
            hilet margin = theme<prefix / "margin", int>{}(this);
            hilet spacing = theme<prefix>.int_spacing_horizontal(this);
            hilet size = extent2i{theme_size * 3 + 2 * margin + 2 * spacing, theme_size + 2 * spacing};
            return {size, size, size};

        } else {
            hi_no_default();
        }
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(layout, context)) {
            auto extent = context.size();
            auto y = context.height() - extent.height();

            if (operating_system::current == operating_system::windows) {
                closeRectangle =
                    aarectanglei{point2i(extent.width() * 2 / 3, y), extent2i{extent.width() * 1 / 3, extent.height()}};

                maximizeRectangle =
                    aarectanglei{point2i(extent.width() * 1 / 3, y), extent2i{extent.width() * 1 / 3, extent.height()}};

                minimizeRectangle = aarectanglei{point2i(0, y), extent2i{extent.width() * 1 / 3, extent.height()}};

            } else if (operating_system::current == operating_system::macos) {
                hilet size = theme<prefix / "macos.size", int>{}(this);
                hilet margin = theme<prefix / "margin", int>{}(this);
                hilet spacing = theme<prefix>.int_spacing_horizontal(this);

                closeRectangle = aarectanglei{point2i(margin, (extent.height() - size) / 2), extent2i{size, size}};

                minimizeRectangle =
                    aarectanglei{point2i(margin + size + spacing, (extent.height() - size) / 2), extent2i{size, size}};

                maximizeRectangle = aarectanglei{
                    point2i(margin + size + spacing + size + spacing, (extent.height() - size) / 2), extent2i{size, size}};
            } else {
                hi_no_default();
            }

            closeWindowGlyph = find_glyph(hikogui_icon::CloseWindow);
            minimizeWindowGlyph = find_glyph(hikogui_icon::MinimizeWindow);

            if (operating_system::current == operating_system::windows) {
                maximizeWindowGlyph = find_glyph(hikogui_icon::MaximizeWindowMS);
                restoreWindowGlyph = find_glyph(hikogui_icon::RestoreWindowMS);

            } else if (operating_system::current == operating_system::macos) {
                maximizeWindowGlyph = find_glyph(hikogui_icon::MaximizeWindowMacOS);
                restoreWindowGlyph = find_glyph(hikogui_icon::RestoreWindowMacOS);
            } else {
                hi_no_default();
            }

            hilet glyph_size = operating_system::current == operating_system::macos ?
                theme<prefix / "macos.icon.size", float>{}(this) :
                theme<prefix / "windows.icon.size", float>{}(this);

            hilet closeWindowGlyphBB = narrow_cast<aarectanglei>(closeWindowGlyph.get_bounding_rectangle() * glyph_size);
            hilet minimizeWindowGlyphBB = narrow_cast<aarectanglei>(minimizeWindowGlyph.get_bounding_rectangle() * glyph_size);
            hilet maximizeWindowGlyphBB = narrow_cast<aarectanglei>(maximizeWindowGlyph.get_bounding_rectangle() * glyph_size);
            hilet restoreWindowGlyphBB = narrow_cast<aarectanglei>(restoreWindowGlyph.get_bounding_rectangle() * glyph_size);

            closeWindowGlyphRectangle = align(closeRectangle, closeWindowGlyphBB, alignment::middle_center());
            minimizeWindowGlyphRectangle = align(minimizeRectangle, minimizeWindowGlyphBB, alignment::middle_center());
            maximizeWindowGlyphRectangle = align(maximizeRectangle, maximizeWindowGlyphBB, alignment::middle_center());
            restoreWindowGlyphRectangle = align(maximizeRectangle, restoreWindowGlyphBB, alignment::middle_center());
        }
    }

    void draw(widget_draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout)) {
            if (operating_system::current == operating_system::macos) {
                drawMacOS(context);

            } else if (operating_system::current == operating_system::windows) {
                drawWindows(context);

            } else {
                hi_no_default();
            }
        }
    }

    bool handle_event(gui_event const& event) noexcept override
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
                    switch (layout.window_size_state) {
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

    [[nodiscard]] hitbox hitbox_test(point2i position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (*mode >= widget_mode::partial and layout.contains(position) and
            (closeRectangle.contains(position) or minimizeRectangle.contains(position) or maximizeRectangle.contains(position))) {
            return hitbox{id, layout.elevation, hitbox_type::button};
        } else {
            return {};
        }
    }
    /// @endprivatesection
private:
    aarectanglei closeRectangle;
    aarectanglei minimizeRectangle;
    aarectanglei maximizeRectangle;

    font_book::font_glyph_type closeWindowGlyph;
    font_book::font_glyph_type minimizeWindowGlyph;
    font_book::font_glyph_type maximizeWindowGlyph;
    font_book::font_glyph_type restoreWindowGlyph;

    aarectanglei closeWindowGlyphRectangle;
    aarectanglei minimizeWindowGlyphRectangle;
    aarectanglei maximizeWindowGlyphRectangle;
    aarectanglei restoreWindowGlyphRectangle;

    bool hoverClose = false;
    bool hoverMinimize = false;
    bool hoverMaximize = false;

    bool pressedClose = false;
    bool pressedMinimize = false;
    bool pressedMaximize = false;

    void drawMacOS(widget_draw_context const& context) noexcept
    {
        context.draw_box(
            layout,
            closeRectangle,
            theme<prefix / "windows.close.fill.color", color>{}(this),
            corner_radii{closeRectangle.height() / 2.0f});

        context.draw_box(
            layout,
            minimizeRectangle,
            theme<prefix / "windows.minimize.fill.color", color>{}(this),
            corner_radii{minimizeRectangle.height() / 2.0f});

        context.draw_box(
            layout,
            maximizeRectangle,
            theme<prefix / "windows.maximize.fill.color", color>{}(this),
            corner_radii{maximizeRectangle.height() / 2.0f});

        if (*hover) {
            context.draw_glyph(
                layout,
                translate_z(0.1f) * narrow_cast<aarectangle>(closeWindowGlyphRectangle),
                closeWindowGlyph,
                theme<prefix / "windows.close.icon.color", color>{}(this));
            context.draw_glyph(
                layout,
                translate_z(0.1f) * narrow_cast<aarectangle>(minimizeWindowGlyphRectangle),
                minimizeWindowGlyph,
                theme<prefix / "windows.minimize.icon.color", color>{}(this));

            if (layout.window_size_state == gui_window_size::maximized) {
                context.draw_glyph(
                    layout,
                    translate_z(0.1f) * narrow_cast<aarectangle>(restoreWindowGlyphRectangle),
                    restoreWindowGlyph,
                    theme<prefix / "windows.maximize.icon.color", color>{}(this));
            } else {
                context.draw_glyph(
                    layout,
                    translate_z(0.1f) * narrow_cast<aarectangle>(maximizeWindowGlyphRectangle),
                    maximizeWindowGlyph,
                    theme<prefix / "windows.maximize.icon.color", color>{}(this));
            }
        }
    }

    void drawWindows(widget_draw_context const& context) noexcept
    {
        context.draw_box(layout, closeRectangle, theme<prefix / "windows.close.fill.color", color>{}(this));
        context.draw_box(layout, minimizeRectangle, theme<prefix / "windows.minimize.fill.color", color>{}(this));
        context.draw_box(layout, maximizeRectangle, theme<prefix / "windows.maximize.fill.color", color>{}(this));

        context.draw_glyph(
            layout,
            translate_z(0.1f) * narrow_cast<aarectangle>(closeWindowGlyphRectangle),
            closeWindowGlyph,
            theme<prefix / "windows.close.icon.color", color>{}(this));

        context.draw_glyph(
            layout,
            translate_z(0.1f) * narrow_cast<aarectangle>(minimizeWindowGlyphRectangle),
            minimizeWindowGlyph,
            theme<prefix / "windows.minimize.icon.color", color>{}(this));

        if (layout.window_size_state == gui_window_size::maximized) {
            context.draw_glyph(
                layout,
                translate_z(0.1f) * narrow_cast<aarectangle>(restoreWindowGlyphRectangle),
                restoreWindowGlyph,
                theme<prefix / "windows.maximize.icon.color", color>{}(this));
        } else {
            context.draw_glyph(
                layout,
                translate_z(0.1f) * narrow_cast<aarectangle>(maximizeWindowGlyphRectangle),
                maximizeWindowGlyph,
                theme<prefix / "windows.maximize.icon.color", color>{}(this));
        }
    }
};

}} // namespace hi::v1
