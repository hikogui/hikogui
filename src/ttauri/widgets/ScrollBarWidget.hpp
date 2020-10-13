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
            ttlet thickness = Theme::margin * 2;

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

            // Calculate the position of the slider.
            ttlet slider_offset = *offset * travel_vs_hidden_content_ratio();

            if constexpr (is_vertical) {
                slider_rectangle = aarect{rectangle().x(), rectangle().y() + slider_offset, rectangle().width(), slider_length()};
            } else {
                slider_rectangle =
                    aarect{rectangle().x() + slider_offset, rectangle().y(), slider_length(), rectangle().height()};
            }
        }

        return Widget::updateLayout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        draw_rails(context);

        if (hidden_content() != 0.0f) {
            draw_slider(context);
        }
        Widget::draw(std::move(context), display_time_point);
    }

    HitBox hitBoxTest(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        ttlet position = fromWindowTransform * window_position;

        if (_window_clipping_rectangle.contains(window_position) && slider_rectangle.contains(position) && hidden_content() != 0.0f) {
            return HitBox{this, _draw_layer};
        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] bool handleMouseEvent(MouseEvent const &event) noexcept
    {
        ttlet lock = std::scoped_lock(mutex);

        if (Widget::handleMouseEvent(event)) {
            return true;

        } else if (event.cause.leftButton) {
            switch (event.type) {
                using enum MouseEvent::Type;
            case ButtonDown:
                // Record the original scroll-position before the drag starts.
                offset_before_drag = *offset;
                break;
        
            case Drag: {
                // The distance the slider has to move relative to the slider position at the
                // start of the drag.
                ttlet slider_movement = is_vertical ? event.delta().y() : event.delta().x();
                ttlet content_movement = slider_movement * hidden_content_vs_travel_ratio();
                offset = offset_before_drag + content_movement;
            } break;
        
            default:;
            }
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

    float offset_before_drag;

    [[nodiscard]] float rail_length() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        return is_vertical ? rectangle().height() : rectangle().width();
    }

    [[nodiscard]] float slider_length() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        ttlet content_aperture_ratio = *aperture / *content;
        return std::max(rail_length() * content_aperture_ratio, Theme::smallSize * 2.0f);
    }

    /** The amount of travel that the slider can make.
     */
    [[nodiscard]] float slider_travel_range() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        return rail_length() - slider_length();
    }

    /** The amount of content hidden from view.
     */
    [[nodiscard]] float hidden_content() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        return *content - *aperture;
    }

    /** Get the ratio of the hidden content vs the slider travel range.
     * We can not simply take the ratio of content vs rail length, because
     * there is a minimum slider length.
     */
    [[nodiscard]] float hidden_content_vs_travel_ratio() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        ttlet _slider_travel_range = slider_travel_range();
        return _slider_travel_range != 0.0f ? hidden_content() / _slider_travel_range : 0.0f;
    }

    /** Get the ratio of the slider travel range vs hidden content.
     * We can not simply take the ratio of content vs rail length, because
     * there is a minimum slider length.
     */
    [[nodiscard]] float travel_vs_hidden_content_ratio() const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        ttlet _hidden_content = hidden_content();
        return _hidden_content != 0.0f ? slider_travel_range() / _hidden_content : 0.0f;
    }

    void draw_rails(DrawContext context) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        context.color = theme->fillColor(_semantic_layer);
        context.fillColor = theme->fillColor(_semantic_layer);
        if constexpr (is_vertical) {
            context.cornerShapes = vec{rectangle().width() * 0.5f};
        } else {
            context.cornerShapes = vec{rectangle().height() * 0.5f};
        }
        context.drawBoxIncludeBorder(rectangle());
    }

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
