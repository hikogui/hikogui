// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "audio_block.hpp"

namespace tt {
class audio_system;

class audio_system_delegate {
public:
    audio_system_delegate() noexcept = default;

    /*! Called when the device list has changed.
     * This can happen when external devices are connected or disconnected.
     */
    virtual void audio_device_list_changed(audio_system &self) = 0;
};

}