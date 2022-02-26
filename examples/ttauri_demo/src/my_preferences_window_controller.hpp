// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/GUI/gui_window_delegate.hpp"
#include "ttauri/audio/audio_system_delegate.hpp"
#include "ttauri/audio/audio_device_id.hpp"
#include "ttauri/audio/speaker_mapping.hpp"
#include "ttauri/observable.hpp"
#include "ttauri/label.hpp"
#include "ttauri/preferences.hpp"

class my_preferences_window_controller :
    public tt::gui_window_delegate,
    public tt::audio_system_delegate {
public:
    my_preferences_window_controller(tt::preferences &preferences) noexcept
    {
        preferences.add("audio_output_device_id", audio_output_device_id);
        preferences.add("audio_output_exclusive", audio_output_exclusive);
        preferences.add("audio_output_sample_rate", audio_output_sample_rate);
        preferences.add("audio_output_speaker_mapping", audio_output_speaker_mapping, tt::speaker_mapping::none);
    }

    void init(tt::gui_window& window) noexcept override;
    void audio_device_list_changed(tt::audio_system& system) noexcept;

private:
    tt::observable<tt::audio_device_id> audio_output_device_id;
    tt::observable<bool> audio_output_exclusive;
    tt::observable<double> audio_output_sample_rate;
    tt::observable<tt::speaker_mapping> audio_output_speaker_mapping;

    tt::observable<int> tab_index = 0;
    tt::observable<bool> toggleValue;
    tt::observable<int> radioValue = 0;
    tt::observable<std::vector<std::pair<tt::audio_device_id, tt::label>>> _audio_device_list;

    tt::observable<std::vector<std::pair<std::string, tt::label>>> _theme_list;
    tt::observable<std::string> _selected_theme;

    void init_audio_tab(tt::grid_widget &grid) noexcept;
    void init_theme_tab(tt::grid_widget &grid) noexcept;
    void init_license_tab(tt::grid_widget &grid) noexcept;
};

