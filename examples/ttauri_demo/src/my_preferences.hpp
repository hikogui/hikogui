// Copyright 2020, 2021 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/preferences.hpp"
#include "ttauri/observable.hpp"
#include "ttauri/audio/audio_device_id.hpp"

class my_preferences : public tt::preferences {
public:
    using super = tt::preferences;

    tt::observable<tt::audio_device_id> audio_output_device_id;
    tt::observable<bool> audio_output_exclusive;
    tt::observable<double> audio_output_sample_rate;
    tt::observable<tt::speaker_mapping> audio_output_speaker_mapping;

    my_preferences(tt::URL location) noexcept : super(location)
    {
        register_item("audio_output_device_id", audio_output_device_id);
        register_item("audio_output_exclusive", audio_output_exclusive);
        register_item("audio_output_sample_rate", audio_output_sample_rate);
        register_item("audio_output_speaker_mapping", audio_output_speaker_mapping, tt::speaker_mapping::direct);
    }
    
};

inline std::unique_ptr<my_preferences> g_my_preferences;

