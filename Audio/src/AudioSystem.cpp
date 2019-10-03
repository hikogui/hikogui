// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Audio/AudioSystem.hpp"

namespace TTauri::Audio {

AudioSystem::AudioSystem(std::shared_ptr<AudioSystemDelegate> delegate) :
    delegate(std::move(delegate))
{
}

AudioSystem::~AudioSystem()
{
}

bool AudioSystem::hasDeviceWithId(std::string id) const noexcept {
    for (let &device: devices) {
        if (device->id == id) {
            return true;
        }
    }
    return false;
}

}