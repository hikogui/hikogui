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

class ToolbarButtonWidget : public Widget {
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
            requestConstraint = true;
        });

        // Toolbar buttons hug the toolbar and neighbour widgets.
        margin = 0.0f;
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

    [[nodiscard]] bool updateConstraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (Widget::updateConstraints()) {
            icon_cell = (*icon).makeCell();
            ttlet width = Theme::toolbarDecorationButtonWidth;
            ttlet height = Theme::toolbarHeight;
            _preferred_size = {vec{width, height}, vec{width, std::numeric_limits<float>::infinity()}};
            return true;
        } else {
            return false;
        }
    }

    void drawBackground(DrawContext context) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (pressed) {
            context.fillColor = theme->fillColor(nestingLevel() + 1);
        } else if (hover) {
            context.fillColor = theme->fillColor(nestingLevel());
        } else {
            context.fillColor = theme->fillColor(nestingLevel() - 1);
        }
        context.drawFilledQuad(rectangle());
    }

    void drawIcon(DrawContext context) noexcept
    {
        context.transform = mat::T(0.0f, 0.0f, 0.0001f) * context.transform;
        if (*enabled) {
            context.color = theme->foregroundColor;
        }
        icon_cell->draw(context, rectangle(), Alignment::MiddleCenter);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        drawBackground(drawContext);
        drawIcon(drawContext);
        Widget::draw(drawContext, displayTimePoint);
    }

    void handleMouseEvent(MouseEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);

        Widget::handleMouseEvent(event);

        if (*enabled) {
            if (compare_then_assign(pressed, static_cast<bool>(event.down.leftButton))) {
                window.requestRedraw = true;
            }

            if (event.type == MouseEvent::Type::ButtonUp && event.cause.leftButton) {
                ttlet position = fromWindowTransform * event.position;
                if (rectangle().contains(position)) {
                    tt_assert2(_delegate, "Delegate on ToolbarButtonWidget was not set");
                    run_from_main_loop(_delegate);
                }
            }
        }
    }

    [[nodiscard]] HitBox hitBoxTest(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        ttlet position = fromWindowTransform * window_position;

        if (rectangle().contains(position)) {
            return HitBox{this, elevation, *enabled ? HitBox::Type::Button : HitBox::Type::Default};
        } else {
            return HitBox{};
        }
    }

private:
    bool pressed = false;

    std::unique_ptr<ImageCell> icon_cell;

    delegate_type _delegate;
};

} // namespace tt
