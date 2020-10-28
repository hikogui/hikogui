// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "abstract_button_widget.hpp"
#include "../Path.hpp"
#include "../text/FontGlyphIDs.hpp"
#include "../text/ElusiveIcons.hpp"
#include "../text/TTauriIcons.hpp"
#include "../GUI/DrawContext.hpp"
#include <memory>
#include <string>
#include <array>
#include <variant>

namespace tt {

class ToolbarButtonWidget final : public abstract_button_widget {
public:
    observable<Image> icon;

    ToolbarButtonWidget(Window &window, std::shared_ptr<Widget> parent) noexcept :
        abstract_button_widget(window, parent)
    {
        icon_callback = this->icon.subscribe([this](auto...) {
            request_reconstrain = true;
        });

        // Toolbar buttons hug the toolbar and neighbour widgets.
        p_margin = 0.0f;
    }

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (Widget::update_constraints()) {
            icon_cell = (*icon).makeCell();
            ttlet width = Theme::toolbarDecorationButtonWidth;
            ttlet height = Theme::toolbarHeight;
            p_preferred_size = {vec{width, height}, vec{width, std::numeric_limits<float>::infinity()}};
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        need_layout |= std::exchange(request_relayout, false);
        return Widget::update_layout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
        drawBackground(context);
        drawIcon(context);
        Widget::draw(std::move(context), display_time_point);
    }

private:
    typename decltype(icon)::callback_ptr_type icon_callback;
    std::unique_ptr<ImageCell> icon_cell;

    void drawBackground(DrawContext context) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (_pressed) {
            context.fillColor = theme->fillColor(p_semantic_layer + 1);
        } else if (hover) {
            context.fillColor = theme->fillColor(p_semantic_layer);
        } else {
            context.fillColor = theme->fillColor(p_semantic_layer - 1);
        }
        context.drawFilledQuad(rectangle());
    }

    void drawIcon(DrawContext context) noexcept
    {
        context.transform = mat::T(0.0f, 0.0f, 0.1f) * context.transform;
        if (*enabled) {
            context.color = theme->foregroundColor;
        }
        icon_cell->draw(context, rectangle(), Alignment::MiddleCenter);
    }
};

} // namespace tt
