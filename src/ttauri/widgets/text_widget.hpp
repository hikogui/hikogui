// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "../GFX/draw_context.hpp"
#include "../observable.hpp"
#include "../alignment.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class text_widget final : public widget {
public:
    using super = widget;

    observable<l10n> text;
    observable<alignment> alignment = alignment::middle_center;
    observable<theme_text_style> text_style = theme_text_style::label;

    text_widget(
        gui_window &window, widget *parent) noexcept :
        super(window, parent)
    {
    }

    template<typename Text>
        text_widget(
        gui_window &window, widget *parent,
        Text &&text) noexcept :
        text_widget(window, parent)
    {
        text = std::forward<Text>(text);
    }


    ~text_widget();

    void init() noexcept override;

    [[nodiscard]] bool constrain(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;

    [[nodiscard]] void layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override;

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override;

private:
    decltype(text)::callback_ptr_type _text_callback;

    shaped_text _shaped_text;
    matrix2 _shaped_text_transform;
};

} // namespace tt
