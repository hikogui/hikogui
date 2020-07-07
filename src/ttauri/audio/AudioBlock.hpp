// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/foundation/hires_utc_clock.hpp"
#include <vector>

namespace tt {

enum class SampleFormat {
    PCM_SignedInteger,
    PCM_SignedInteger_LE,
    PCM_SignedInteger_BE,
    PCM_Float,
    PCM_Float_LE,
    PCM_Float_BE,
};

struct AudioBuffer {
    //! Number of interleaved channels in the sample data.
    size_t numberOfChannels;

    //! Sample data.
    void *samples;
};

struct AudioBlock {
    /*! The sample position of the first sample in this block
     * since the start of the capture/render session.
     */
    uint64_t samplePosition;

    /*! Timestamp when the sample first sample in this block
     * was captured at the audio device input or when the first sample
     * in this block will appear at the audio device output.
     */
    hires_utc_clock::time_point timestamp;

    // XXX More timestamps received directly from an audio interface.

    /*! Number of samples in this audio block.
     */
    size_t numberOfSamples;

    /*! Sample rate of the word clock attached to the audio interface
     * For example in some situation on film sets the audio interface
     * sample rate is overdriven to 48048 Hz.
     */
    double wordClockSampleRate;

    /*! Sample rate to what the audio device is configured to.
     */
    double deviceSampleRate;

    /*! Number of bits in each sample.
     */
    size_t numberOfBitsPerSample;

    /*! Number of bytes to jump to the next sample.
     */
    size_t stridePerSample;

    /*! Format of the samples in this block.
     */
    SampleFormat sampleFormat;

    /*! The data in the buffer contains a glitch.
     * Examples of how glitches could happen:
     *  * CRC error caused by a bad USB/Firewire/Ethernet cable.
     *  * Sample rate of the word clock or digital audio input
     *    and the sample rate of the audio device is too far off.
     */
    bool containsGlitch;

    /*! The timestamp in this block is bad.
     */
    bool badTimestamp;

    /*! This block of audio is silent.
     */
    bool silent;

    /*! A list of sample data buffers.
     * Each buffer contains a set of interleaved channels.
     *
     * Some devices create a single buffer with all channels interleaved.
     * Some devices create one non-interleaved buffer for each channel.
     * Some devices have different sets of inputs, each interleaved, for
     * example 8 digital inputs interleaved in one buffer, and 10 analogue
     * inputs interleaved in the second buffer.
     */
    std::vector<AudioBuffer> buffers;
};


}