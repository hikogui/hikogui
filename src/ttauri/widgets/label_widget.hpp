// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "text_widget.hpp"
#include "icon_widget.hpp"
#include "label_delegate.hpp"
#include "../alignment.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class label_widget final : public widget {
public:
    using super = widget;

    ~label_widget();

    label_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<label_delegate> delegate,
        alignment alignment = alignment::middle_right,
        text_style text_style = theme::global->labelStyle) noexcept;

    template<typename Label>
    label_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<label_delegate> delegate,
        alignment alignment,
        text_style text_style,
        Label &&label) noexcept :
        super(window, std::move(parent), std::move(delegate)), _alignment(alignment), _text_style(text_style)
    {
        set_label(std::forward<Label>(label));
    }

    template<typename Label>
    label_widget(gui_window &window, std::shared_ptr<widget> parent, Label &&label) noexcept :
        label_widget(
            window,
            std::move(parent),
            std::make_shared<label_delegate>(),
            alignment::middle_right,
            theme::global->labelStyle,
            std::forward<Label>(label))
    {
    }


    void init() noexcept override;

    tt::label label() const noexcept;

    void set_label(observable<tt::label> label) noexcept;

    void set_label(l10n label) noexcept;

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;

    [[nodiscard]] void update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override;

private:
    float _icon_size;
    float _inner_margin;

    std::shared_ptr<icon_widget> _icon_widget;
    std::shared_ptr<text_widget> _text_widget;
    text_style _text_style;
    alignment _alignment;
};

} // namespace tt
