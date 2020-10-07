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

    bool updateConstraints() noexcept override;
    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override;
    bool handleMouseEvent(MouseEvent const &event) noexcept override;
    void handleCommand(command command) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec window_position) const noexcept override;

    [[nodiscard]] bool acceptsFocus() const noexcept override
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
