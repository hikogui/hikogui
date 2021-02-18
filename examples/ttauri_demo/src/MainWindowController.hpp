// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/GUI/gui_window_delegate.hpp"
#include "ttauri/widgets/widgets.hpp"

namespace ttauri_demo {

class MainWindowController : public tt::gui_window_delegate {
public:
    void init(tt::gui_window &window) noexcept override;

    typename tt::button_widget<bool>::callback_ptr_type preferences_button_callback;
};

}