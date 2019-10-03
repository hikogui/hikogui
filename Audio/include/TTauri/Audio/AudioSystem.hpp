// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Audio/AudioSystemDelegate.hpp"
#include "TTauri/Audio/AudioDevice.hpp"
#include <vector>
#include <memory>

namespace TTauri::Audio {

/*! An system of audio devices.
 * Systems are for example: Window Audio Session API (WASAPI), ASIO, Apple CoreAudio
 */
class AudioSystem {
protected:
    std::shared_ptr<AudioSystemDelegate> delegate;
    std::vector<std::unique_ptr<AudioDevice>> devices;

public:
    AudioSystem(std::shared_ptr<AudioSystemDelegate> delegate);
    virtual ~AudioSystem();

    size_t size() const noexcept {
        return devices.size();
    }

    auto begin() noexcept {
        return devices.begin();
    }

    auto end() noexcept {
        return devices.end();
    }

    bool hasDeviceWithId(std::string id) const noexcept;
};

}