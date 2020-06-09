// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Audio/AudioBlock.hpp"

namespace TTauri {

class AudioDeviceDelegate {
public:
    AudioDeviceDelegate();
    virtual ~AudioDeviceDelegate() = 0;

    AudioDeviceDelegate(AudioDeviceDelegate const &) = delete;
    AudioDeviceDelegate &operator=(AudioDeviceDelegate const &) = delete;
    AudioDeviceDelegate(AudioDeviceDelegate &&) = delete;
    AudioDeviceDelegate &operator=(AudioDeviceDelegate &&) = delete;

    /*! Process a block of samples.
     *
     * \param inputBlock Samples captured from the audio device.
     * \param outputBlock Samples rendered to the audio device.
     */
    virtual void processAudio(AudioBlock const &inputBlock, AudioBlock &outputBlock, hires_utc_clock::time_point current_time) noexcept = 0;
};

}