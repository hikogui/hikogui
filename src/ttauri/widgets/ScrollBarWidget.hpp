// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "widget.hpp"
#include "../GUI/draw_context.hpp"
#include "../observable.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<bool IsVertical>
class ScrollBarWidget final : public widget {
public:
    using super = widget;

    static constexpr bool is_vertical = IsVertical;

    template<typename Content, typename Aperture, typename Offset>
    ScrollBarWidget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        Content &&content,
        Aperture &&aperture,
        Offset &&offset) noexcept :
        widget(window, parent),
        content(std::forward<Content>(content)),
        aperture(std::forward<Aperture>(aperture)),
        offset(std::forward<Offset>(offset))
    {
        _content_callback = this->content.subscribe([this](auto...) {
            this->window.requestLayout = true;
        });
        _aperture_callback = this->aperture.subscribe([this](auto...) {
            this->window.requestLayout = true;
        });
        _offset_callback = this->offset.subscribe([this](auto...) {
            this->window.requestLayout = true;
        });
    }

    ~ScrollBarWidget() {}

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            ttlet minimum_length = theme::global->width; // even for vertical bars.

            if constexpr (is_vertical) {
                _preferred_size = interval_vec2{
                    f32x4{theme::global->scroll_bar_thickness, minimum_length},
                    f32x4{theme::global->scroll_bar_thickness, std::numeric_limits<float>::max()}};
            } else {
                _preferred_size = interval_vec2{
                    f32x4{minimum_length, theme::global->scroll_bar_thickness},
                    f32x4{std::numeric_limits<float>::max(), theme::global->scroll_bar_thickness}};
            }

            _preferred_base_line = {};
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            tt_axiom(*content != 0.0f);

            // Calculate the position of the slider.
            ttlet slider_offset = *offset * travel_vs_hidden_content_ratio();

            if constexpr (is_vertical) {
                slider_rectangle = aarect{rectangle().x(), rectangle().y() + slider_offset, rectangle().width(), slider_length()};
            } else {
                slider_rectangle =
                    aarect{rectangle().x() + slider_offset, rectangle().y(), slider_length(), rectangle().height()};
            }
        }

        widget::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, this->window_clipping_rectangle()) && visible()) {
            draw_rails(context);
            draw_slider(context);
        }
        widget::draw(std::move(context), display_time_point);
    }

    HitBox hitbox_test(f32x4 window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        ttlet position = _from_window_transform * window_position;

        if (window_clipping_rectangle().contains(window_position) && slider_rectangle.contains(position) && visible()) {
            return HitBox{weak_from_this(), _draw_layer};
        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] bool handle_mouse_event(MouseEvent const &event) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        auto handled = widget::handle_mouse_event(event);
        
        if (event.cause.leftButton) {
            handled = true;
            
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
        }
        return handled;
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        return false;
    }

    /** Is the scrollbar visible.
     * When the content is the same size as the scroll-view then
     * the scrollbar becomes invisible.
     */
    [[nodiscard]] bool visible() const noexcept {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return hidden_content() >= 1.0f;
    }

private:
    observable<float> offset;
    observable<float> aperture;
    observable<float> content;

    typename decltype(offset)::callback_ptr_type _offset_callback;
    typename decltype(aperture)::callback_ptr_type _aperture_callback;
    typename decltype(content)::callback_ptr_type _content_callback;

    aarect slider_rectangle;

    float offset_before_drag;

    [[nodiscard]] float rail_length() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return is_vertical ? rectangle().height() : rectangle().width();
    }

    [[nodiscard]] float slider_length() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        ttlet content_aperture_ratio = *aperture / *content;
        return std::max(rail_length() * content_aperture_ratio, theme::global->smallSize * 2.0f);
    }

    /** The amount of travel that the slider can make.
     */
    [[nodiscard]] float slider_travel_range() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return rail_length() - slider_length();
    }

    /** The amount of content hidden from view.
     */
    [[nodiscard]] float hidden_content() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return *content - *aperture;
    }

    /** Get the ratio of the hidden content vs the slider travel range.
     * We can not simply take the ratio of content vs rail length, because
     * there is a minimum slider length.
     */
    [[nodiscard]] float hidden_content_vs_travel_ratio() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        ttlet _slider_travel_range = slider_travel_range();
        return _slider_travel_range != 0.0f ? hidden_content() / _slider_travel_range : 0.0f;
    }

    /** Get the ratio of the slider travel range vs hidden content.
     * We can not simply take the ratio of content vs rail length, because
     * there is a minimum slider length.
     */
    [[nodiscard]] float travel_vs_hidden_content_ratio() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        ttlet _hidden_content = hidden_content();
        return _hidden_content != 0.0f ? slider_travel_range() / _hidden_content : 0.0f;
    }

    void draw_rails(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        context.color = theme::global->fillColor(_semantic_layer);
        context.fill_color = theme::global->fillColor(_semantic_layer);
        if constexpr (is_vertical) {
            context.corner_shapes = f32x4::broadcast(rectangle().width() * 0.5f);
        } else {
            context.corner_shapes = f32x4::broadcast(rectangle().height() * 0.5f);
        }
        context.draw_box_with_border_inside(rectangle());
    }

    void draw_slider(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        context.color = theme::global->fillColor(_semantic_layer + 1);
        context.fill_color = theme::global->fillColor(_semantic_layer + 1);
        context.transform = mat::T{0.0f, 0.0f, 0.1f} * context.transform;
        if constexpr (is_vertical) {
            context.corner_shapes = f32x4::broadcast(slider_rectangle.width() * 0.5f);
        } else {
            context.corner_shapes = f32x4::broadcast(slider_rectangle.height() * 0.5f);
        }
        context.draw_box_with_border_inside(slider_rectangle);
    }
};

} // namespace tt
