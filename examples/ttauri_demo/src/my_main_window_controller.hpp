// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/GUI/gui_window_delegate.hpp"
#include "ttauri/widgets/widgets.hpp"

class my_main_window_controller : public tt::gui_window_delegate {
public:
    using super = tt::gui_window_delegate;

    static inline std::shared_ptr<my_main_window_controller> global;

    void init(tt::gui_window &window) noexcept override;

    typename tt::momentary_button_widget::callback_ptr_type preferences_button_callback;
};
