// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "audio_block.hpp"

namespace tt {

class audio_device_delegate {
public:
    audio_device_delegate();
    virtual ~audio_device_delegate() = 0;

    /** Process a block of samples.
     *
     * @param inputBlock Samples captured from the audio device.
     * @param outputBlock Samples rendered to the audio device.
     * @param current_time The current time according to the audio clock.
     */
    virtual void process_audio(audio_block const &inputBlock, audio_block &outputBlock, hires_utc_clock::time_point current_time) noexcept = 0;
};

}
