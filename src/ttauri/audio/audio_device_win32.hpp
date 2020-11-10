// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "audio_device.hpp"

namespace tt {

/*! A class representing an audio device on the system.
*/
class audio_device_win32 : public audio_device {
public:
    audio_device_win32(void *device);
    ~audio_device_win32();

    std::string name() const noexcept override;
    std::string device_name() const noexcept override;
    std::string end_point_name() const noexcept override;
    audio_device_state state() const noexcept override;

    static std::string get_id_from_device(void *device) noexcept;

private:
    void *_device;
    void *_property_store;
};

}