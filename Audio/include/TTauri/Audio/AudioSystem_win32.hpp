// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Audio/AudioSystem.hpp"
#include "TTauri/Audio/AudioSystemDelegate.hpp"
#include <memory>

namespace TTauri::Audio {

class AudioSystem_win32: AudioSystem {
private:
    void *deviceEnumerator;

public:
    AudioSystem_win32(std::shared_ptr<AudioSystemDelegate> delegate);
    virtual ~AudioSystem_win32();

    void updateDeviceList() noexcept;

};

}