// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "AudioSystemDelegate.hpp"
#include "AudioDevice.hpp"
#include <vector>
#include <memory>

namespace tt {

/*! An system of audio devices.
 * Systems are for example: Window Audio Session API (WASAPI), ASIO, Apple CoreAudio
 */
class AudioSystem {
protected:
    AudioSystemDelegate *delegate;
    std::vector<std::unique_ptr<AudioDevice>> devices;

public:
    AudioSystem(AudioSystemDelegate *delegate);
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

    virtual void initialize() noexcept;

    bool hasDeviceWithId(std::string id) const noexcept;
};

inline AudioSystem *audioSystem = nullptr;

}