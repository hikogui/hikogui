// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "audio_preferences_controller.hpp"
#include "license_preferences_controller.hpp"
#include "ttauri/GUI/gui_window_delegate.hpp"
#include "ttauri/observable.hpp"

namespace demo {

class preferences_controller : public std::enable_shared_from_this<preferences_controller>, public tt::gui_window_delegate {
public:
    void init(tt::gui_window &window) noexcept override;

    void audio_device_list_changed(tt::audio_system &system) noexcept;

private:
    tt::observable<int> tab_index = 0;

    tt::observable<bool> toggleValue;
    tt::observable<int> radioValue = 0;

    tt::observable<std::vector<std::pair<std::string,tt::label>>> _audio_device_list;

    std::shared_ptr<audio_preferences_controller> audio_preferences_controller;
    std::shared_ptr<license_preferences_controller> license_preferences_controller;

    friend class audio_preferences_controller;
    friend class license_preferences_controller;
};

}
