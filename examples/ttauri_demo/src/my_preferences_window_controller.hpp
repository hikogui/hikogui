// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "my_preferences.hpp"
#include "ttauri/GUI/gui_window_delegate.hpp"
#include "ttauri/audio/audio_system_delegate.hpp"
#include "ttauri/widgets/audio_device_configuration_controller.hpp"
#include "ttauri/observable.hpp"

class my_preferences_window_controller :
    public std::enable_shared_from_this<my_preferences_window_controller>,
    public tt::gui_window_delegate,
    public tt::audio_system_delegate {
public:
    my_preferences_window_controller(my_preferences &preferences) noexcept :
        _preferences(preferences) {}

    void init(tt::gui_window& window) noexcept override;
    void audio_device_list_changed(tt::audio_system& system) noexcept;

private:
    my_preferences &_preferences;

    tt::observable<int> tab_index = 0;
    tt::observable<bool> toggleValue;
    tt::observable<int> radioValue = 0;
    tt::observable<std::vector<std::pair<std::string, tt::label>>> _audio_device_list;
    std::shared_ptr<tt::audio_device_configuration_controller> _audio_device_configurator;

    void init_audio_tab(tt::grid_layout_widget &grid) noexcept;
    void init_license_tab(tt::grid_layout_widget &grid) noexcept;
};

