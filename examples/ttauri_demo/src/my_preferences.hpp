// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/preferences.hpp"
#include "ttauri/observable.hpp"
#include "ttauri/audio/audio_device_id.hpp"

class my_preferences : public tt::preferences {
public:
    using super = tt::preferences;

    tt::observable<tt::audio_device_id> audio_output_device_id;
    tt::observable<double> sample_rate;

    my_preferences(tt::URL location) noexcept : super(location)
    {
        audio_output_device_id.subscribe(_set_modified_ptr);
    }
    
    void reset() noexcept override
    {
        super::reset();
        audio_output_device_id = {};
    }

    [[nodiscard]] tt::datum serialize() const noexcept override
    {
        auto r = super::serialize();

        //r["audio_output_device_id"] = string_codec<audio_device_id>::serialize(*audio_output_device_id);
        return r;
    }

    void deserialize(tt::datum const &data) noexcept override
    {
        super::deserialize(data);

        //if (auto device_id_string = get_optional_by_path<std::string>("audio_output_device_id")) {
        //    audio_output_device_id = string_codec<audio_device_id>::deserialize(*device_id_string);
        //} else {
        //    audio_output_device_id = {};
        //}
    }

};

inline std::unique_ptr<my_preferences> g_my_preferences;
