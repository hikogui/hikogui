// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "../GUI/draw_context.hpp"
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

    text_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        alignment alignment = alignment::top_left,
        text_style style = theme::global->labelStyle) noexcept :
        super(window, std::move(parent)), _alignment(alignment), _style(style)
    {
    }

    template<typename Text>
        text_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        Text &&text,
        alignment alignment = alignment::top_left,
        text_style style = theme::global->labelStyle) noexcept :
        text_widget(window, std::move(parent), alignment, style)
    {
        text = std::forward<Text>(text);
    }


    ~text_widget();

    void init() noexcept override;

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;

    [[nodiscard]] void update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override;

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override;

private:
    decltype(text)::callback_ptr_type _text_callback;

    alignment _alignment;
    text_style _style;
    shaped_text _shaped_text;
    matrix2 _shaped_text_transform;
};

} // namespace tt
