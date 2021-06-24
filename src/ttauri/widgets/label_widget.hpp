// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "text_widget.hpp"
#include "icon_widget.hpp"
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

    observable<label> label;
    observable<theme_text_style> text_style = theme_text_style::label;
    observable<alignment> alignment = alignment::middle_right;

    ~label_widget();

    label_widget(gui_window &window, widget *parent) noexcept;

    template<typename Label>
    label_widget(gui_window &window, widget *parent, Label &&label) noexcept :
        label_widget(window, parent)
    {
        this->label = std::forward<Label>(label);
    }

    void init() noexcept override;

    [[nodiscard]] bool
    update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;

    [[nodiscard]] void update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override;

private:
    float _icon_size;
    float _inner_margin;

    decltype(label)::callback_ptr_type _label_callback;

    icon_widget *_icon_widget;
    text_widget *_text_widget;
};

} // namespace tt
