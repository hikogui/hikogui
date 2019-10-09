// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Audio/AudioSystem.hpp"

namespace TTauri::Audio {

AudioSystem::AudioSystem(AudioSystemDelegate *delegate) :
    delegate(delegate)
{
}

AudioSystem::~AudioSystem()
{
}

void AudioSystem::initialize() noexcept
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