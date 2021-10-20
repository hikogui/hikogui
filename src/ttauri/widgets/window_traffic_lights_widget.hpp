// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "../graphic_path.hpp"
#include "../text/font_glyph_ids.hpp"
#include <memory>
#include <string>
#include <array>

namespace tt {
struct graphic_path;
}

namespace tt {

class window_traffic_lights_widget final : public widget {
public:
    using super = widget;

    window_traffic_lights_widget(gui_window &window, widget *parent) noexcept;

    /// @privatesection
    [[nodiscard]] float margin() const noexcept override;
    [[nodiscard]] bool constrain(utc_nanoseconds display_time_point, bool need_reconstrain) noexcept override;
    void layout(layout_context const &context, bool need_layout) noexcept override;
    void draw(draw_context const &context) noexcept override;
    bool handle_event(mouse_event const &event) noexcept override;
    [[nodiscard]] hitbox hitbox_test(point3 position) const noexcept override;
    /// @endprivatesection
private:
    static constexpr float GLYPH_SIZE = 5.0f;
    static constexpr float RADIUS = 5.5f;
    static constexpr float DIAMETER = RADIUS * 2.0f;
    static constexpr float MARGIN = 10.0f;
    static constexpr float SPACING = 8.0f;

    /** Size of the glyphs of each icon.
     */
    float _glyph_size;

    aarectangle closeRectangle;
    aarectangle minimizeRectangle;
    aarectangle maximizeRectangle;

    font_glyph_ids closeWindowGlyph;
    font_glyph_ids minimizeWindowGlyph;
    font_glyph_ids maximizeWindowGlyph;
    font_glyph_ids restoreWindowGlyph;

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

    void drawMacOS(draw_context const &context) noexcept;
    void drawWindows(draw_context const &context) noexcept;
};

} // namespace tt
