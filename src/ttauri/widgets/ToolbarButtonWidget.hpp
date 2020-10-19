// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
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

class ToolbarButtonWidget final : public Widget {
public:
    using icon_type = observable<Image>;
    using delegate_type = std::function<void()>;

    icon_type icon;

    ToolbarButtonWidget(
        Window &window,
        Widget *parent,
        icon_type const &icon = icon_type{ElusiveIcon::BanCircle},
        delegate_type const &delegate = delegate_type{}) noexcept :
        Widget(window, parent), icon(icon), _delegate(delegate)
    {
        [[maybe_unused]] ttlet icon_cbid = this->icon.add_callback([this](auto...) {
            request_reconstrain = true;
        });

        // Toolbar buttons hug the toolbar and neighbour widgets.
        p_margin = 0.0f;
    }

    ToolbarButtonWidget(
        Window &window,
        Widget *parent,
        Image const &icon,
        delegate_type const &delegate = delegate_type{}) noexcept :
        ToolbarButtonWidget(window, parent, icon_type{icon}, delegate)
    {
    }

    ToolbarButtonWidget(
        Window &window,
        Widget *parent,
        ElusiveIcon icon,
        delegate_type const &delegate = delegate_type{}) noexcept :
        ToolbarButtonWidget(window, parent, Image{icon}, delegate)
    {
    }

    ToolbarButtonWidget(Window &window, Widget *parent, TTauriIcon icon, delegate_type const &delegate = delegate_type{}) noexcept
        :
        ToolbarButtonWidget(window, parent, Image{icon}, delegate)
    {
    }

    ~ToolbarButtonWidget() {}

    delegate_type &delegate() noexcept
    {
        ttlet lock = std::scoped_lock(mutex);
        return _delegate;
    }

    void setDelegate(delegate_type delegate) noexcept
    {
        ttlet lock = std::scoped_lock(mutex);
        _delegate = delegate;
    }

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

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
        tt_assume(mutex.is_locked_by_current_thread());

        need_layout |= std::exchange(request_relayout, false);
        return Widget::update_layout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        drawBackground(context);
        drawIcon(context);
        Widget::draw(std::move(context), display_time_point);
    }

    bool handle_mouse_event(MouseEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        auto handled = Widget::handle_mouse_event(event);
        
        if (event.cause.leftButton) {
            handled = true;
            if (*enabled) {
                if (compare_then_assign(pressed, static_cast<bool>(event.down.leftButton))) {
                    window.requestRedraw = true;
                }

                if (event.type == MouseEvent::Type::ButtonUp && p_window_rectangle.contains(event.position)) {
                    tt_assert2(_delegate, "Delegate on ToolbarButtonWidget was not set");
                    run_from_main_loop(_delegate);
                }
            }
        }
        return handled;
    }

    [[nodiscard]] HitBox hitbox_test(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);

        if (p_window_clipping_rectangle.contains(window_position)) {
            return HitBox{this, p_draw_layer, *enabled ? HitBox::Type::Button : HitBox::Type::Default};
        } else {
            return HitBox{};
        }
    }

private:
    bool pressed = false;

    std::unique_ptr<ImageCell> icon_cell;

    delegate_type _delegate;

    void drawBackground(DrawContext context) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (pressed) {
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
