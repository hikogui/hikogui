// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"

namespace tt {

class ContainerWidget : public Widget {
public:
    ContainerWidget(Window &window, std::shared_ptr<Widget> parent) noexcept : Widget(window, parent)
    {
        if (parent) {
            // Most containers will not draw itself, only its children.
            ttlet lock = std::scoped_lock(GUISystem_mutex);
            p_semantic_layer = parent->semantic_layer();
        }
        p_margin = 0.0f;
    }

    ~ContainerWidget() {}

    [[nodiscard]] bool update_constraints() noexcept override;
    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override;
    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override;
    [[nodiscard]] HitBox hitbox_test(vec window_position) const noexcept override;
    [[nodiscard]] std::shared_ptr<Widget> next_keyboard_widget(std::shared_ptr<Widget> const &currentKeyboardWidget, bool reverse) const noexcept override;

    /** Remove and deallocate all child widgets.
     */
    void clear() noexcept
    {
        children.clear();
        request_reconstrain = true;
    }

    /** Add a widget directly to this widget.
     * Thread safety: locks.
     */
    virtual std::shared_ptr<Widget> add_widget(std::shared_ptr<Widget> childWidget) noexcept;

    /** Add a widget directly to this widget.
     */
    template<typename T, typename... Args>
    std::shared_ptr<T> make_widget(Args &&... args)
    {
        auto tmp = std::make_shared<T>(window, shared_from_this(), std::forward<Args>(args)...);
        tmp->initialize();
        return std::static_pointer_cast<T>(add_widget(std::move(tmp)));
    }

protected:
    std::vector<std::shared_ptr<Widget>> children;
};

} // namespace tt
