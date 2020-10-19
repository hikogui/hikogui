// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "../cells/TextCell.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class ButtonWidget final : public Widget {
public:
    observable<std::u8string> label;

    ButtonWidget(Window &window, Widget *parent) noexcept;
    ~ButtonWidget();

    bool update_constraints() noexcept override;
    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override;
    bool handle_mouse_event(MouseEvent const &event) noexcept override;
    bool handle_command(command command) noexcept override;

    [[nodiscard]] HitBox hitbox_test(vec window_position) const noexcept override;

    [[nodiscard]] bool accepts_focus() const noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return *enabled;
    }

private:
    bool value = false;
    bool pressed = false;

    std::unique_ptr<TextCell> labelCell;
};

} // namespace tt
