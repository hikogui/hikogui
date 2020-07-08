// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "AudioSystem.hpp"
#include "AudioSystemDelegate.hpp"
#include <memory>

namespace tt {

class AudioSystem_win32: public AudioSystem {
private:
    void *deviceEnumerator;

public:
    AudioSystem_win32(AudioSystemDelegate *delegate);
    virtual ~AudioSystem_win32();

    void initialize() noexcept override;

    void updateDeviceList() noexcept;

};

}