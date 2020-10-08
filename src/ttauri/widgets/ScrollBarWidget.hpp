// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "../cells/TextCell.hpp"
#include "../GUI/DrawContext.hpp"
#include "../observable.hpp"
#include "../text/format10.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<bool IsVertical>
class ScrollBarWidget final : public Widget {
public:
    static constexpr bool is_vertical = IsVertical;

    template<typename Content, typename Aperture, typename Offset>
    ScrollBarWidget(Window &window, Widget *parent, Content &&content, Aperture &&aperture, Offset &&offset) noexcept :
        Widget(window, parent),
        content(std::forward<Content>(content)),
        aperture(std::forward<Aperture>(aperture)),
        offset(std::forward<Offset>(offset))
    {
        [[maybe_unused]] ttlet content_cbid = this->content.add_callback([this](auto...) {
            this->window.requestLayout = true;
        });
        [[maybe_unused]] ttlet aperture_cbid = this->aperture.add_callback([this](auto...) {
            this->window.requestLayout = true;
        });
        [[maybe_unused]] ttlet offset_cbid = this->offset.add_callback([this](auto...) {
            this->window.requestLayout = true;
        });
    }

    ~ScrollBarWidget() {}

    [[nodiscard]] bool updateConstraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (Widget::updateConstraints()) {
            ttlet minimum_length = Theme::width; // even for vertical bars.
            ttlet thickness = Theme::smallSize;

            if constexpr (is_vertical) {
                _preferred_size =
                    interval_vec2{vec{thickness, minimum_length}, vec{thickness, std::numeric_limits<float>::max()}};
            } else {
                _preferred_size =
                    interval_vec2{vec{minimum_length, thickness}, vec{std::numeric_limits<float>::max(), thickness}};
            }

            _preferred_base_line = {};
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        need_layout |= std::exchange(requestLayout, false);
        if (need_layout) {
            tt_assume(*content != 0.0f);

            ttlet rail_length = is_vertical ? rectangle().height() : rectangle().width();

            // Calculate the size of the slider.
            ttlet content_aperture_ratio = *aperture / *content;
            ttlet slider_size = std::max(rail_length * content_aperture_ratio, Theme::smallSize * 2.0f);

            // Calculate the relative offset of both the view and the slider.
            ttlet scroll_space = *content - *aperture;
            ttlet relative_offset = *offset / scroll_space;

            // Calculate the position of the slider.
            ttlet slide_space = rail_length - slider_size;
            ttlet slide_offset = slide_space * relative_offset;

            if constexpr (is_vertical) {
                slider_rectangle = aarect{rectangle().x(), rectangle().y() + slide_offset, rectangle().width(), slider_size};
            } else {
                slider_rectangle = aarect{rectangle().x() + slide_offset, rectangle().y(), slider_size, rectangle().height()};
            }
        }

        return Widget::updateLayout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        context.fillColor = theme->fillColor(_semantic_layer);
        context.drawFilledQuad(rectangle());

        draw_slider(context);
        Widget::draw(std::move(context), display_time_point);
    }

    HitBox hitBoxTest(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        ttlet position = fromWindowTransform * window_position;

        if (_window_clipping_rectangle.contains(window_position) && slider_rectangle.contains(position)) {
            return HitBox{this, _draw_layer};
        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] bool handleMouseEvent(MouseEvent const &event) noexcept
    {
        if (Widget::handleMouseEvent(event)) {
            return true;
        } else if (parent) {
            return parent->handleMouseEvent(event);
        } else {
            return false;
        }
    }

    [[nodiscard]] bool acceptsFocus() const noexcept override
    {
        return false;
    }

private:
    observable<float> offset;
    observable<float> aperture;
    observable<float> content;

    aarect slider_rectangle;

    void draw_slider(DrawContext context) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        context.color = theme->fillColor(_semantic_layer + 1);
        context.fillColor = theme->fillColor(_semantic_layer + 1);
        context.transform = mat::T{0.0f, 0.0f, 0.1f} * context.transform;
        if constexpr (is_vertical) {
            context.cornerShapes = vec{slider_rectangle.width() * 0.5f};
        } else {
            context.cornerShapes = vec{slider_rectangle.height() * 0.5f};
        }
        context.drawBoxIncludeBorder(slider_rectangle);
    }
};

} // namespace tt
