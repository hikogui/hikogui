// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Audio/AudioBlock.hpp"

namespace TTauri::Audio {

class AudioSystemDelegate {
public:
    AudioSystemDelegate();
    virtual ~AudioSystemDelegate() = 0;

    AudioSystemDelegate(AudioSystemDelegate const &) = delete;
    AudioSystemDelegate &operator=(AudioSystemDelegate const &) = delete;
    AudioSystemDelegate(AudioSystemDelegate &&) = delete;
    AudioSystemDelegate &operator=(AudioSystemDelegate &&) = delete;

    /*! Called when the device list has changed.
     * This can happen when external devices are connected or disconnected.
     */
    virtual void deviceListChanged() = 0;
};

}