// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/window_traffic_lights_widget.hpp Defines window_traffic_lights_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "../text/glyph_ids.hpp"
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
class window_traffic_lights_widget final : public widget {
public:
    using super = widget;

    window_traffic_lights_widget(widget *parent) noexcept;

    /// @privatesection
    widget_constraints const& set_constraints(set_constraints_context const &context) noexcept override;
    void set_layout(widget_layout const& context) noexcept override;
    void draw(draw_context const& context) noexcept override;
    bool handle_event(gui_event const& event) noexcept override;
    [[nodiscard]] hitbox hitbox_test(point3 position) const noexcept override;
    /// @endprivatesection
private:
    static constexpr float GLYPH_SIZE = 5.0f;
    static constexpr float RADIUS = 5.5f;
    static constexpr float DIAMETER = RADIUS * 2.0f;
    static constexpr float MARGIN = 10.0f;
    static constexpr float SPACING = 8.0f;

    aarectangle closeRectangle;
    aarectangle minimizeRectangle;
    aarectangle maximizeRectangle;

    glyph_ids closeWindowGlyph;
    glyph_ids minimizeWindowGlyph;
    glyph_ids maximizeWindowGlyph;
    glyph_ids restoreWindowGlyph;

    aarectangle closeWindowGlyphRectangle;
    aarectangle minimizeWindowGlyphRectangle;
    aarectangle maximizeWindowGlyphRectangle;
    aarectangle restoreWindowGlyphRectangle;

    bool hoverClose = false;
    bool hoverMinimize = false;
    bool hoverMaximize = false;

    bool pressedClose = false;
    bool pressedMinimize = false;
    bool pressedMaximize = false;

    void drawMacOS(draw_context const& context) noexcept;
    void drawWindows(draw_context const& context) noexcept;
};

}} // namespace hi::v1
