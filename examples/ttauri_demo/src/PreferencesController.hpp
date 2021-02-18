// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "AudioPreferencesController.hpp"
#include "LicensePreferencesController.hpp"
#include "ttauri/GUI/gui_window_delegate.hpp"
#include "ttauri/observable.hpp"

namespace ttauri_demo {

class PreferencesController : public std::enable_shared_from_this<PreferencesController>, public tt::gui_window_delegate {
public:
    void init(tt::gui_window &window) noexcept override;

    void audio_device_list_changed(tt::audio_system &system) noexcept;

private:
    tt::observable<int> tab_index = 0;

    tt::observable<bool> toggleValue;
    tt::observable<int> radioValue = 0;

    tt::observable<std::vector<std::pair<std::string,tt::label>>> _audio_device_list;

    std::shared_ptr<AudioPreferencesController> audio_preferences_controller;
    std::shared_ptr<LicensePreferencesController> license_preferences_controller;

    friend AudioPreferencesController;
    friend LicensePreferencesController;
};

}