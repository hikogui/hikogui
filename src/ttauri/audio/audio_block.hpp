// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "../hires_utc_clock.hpp"
#include "../required.hpp"
#include <span>

namespace tt {

/** A block of audio data.
 * This represents a block of audio data received from, or to be send to,
 * an audio device.
 * 
 * The samples in this block are always in native floating point format for
 * easy processing. The samples are stored continues for each channel so that
 * processing can be done at a per-channel basis using SSE instructions, for
 * this reason the samples are aligned and in a multiple of the largest vector
 * instructions.
 */
struct audio_block {
    /** The number of samples in a vector.
     */
    static constexpr ssize_t samples_per_vector = 16;

    /** Number of vector of samples in this audio block.
     */
    ssize_t number_of_vectors;

    /** Number of samples in this audio block.
     * The number of samples is always a multiple of `samples_per_vector`.
     */
    [[nodiscard]] ssize_t number_of_samples() const noexcept
    {
        return number_of_vectors * samples_per_vector;
    }

    /** Number of channels in this audio block.
     */
    ssize_t number_of_channels;

    /** Sample data.
     * The samples are organized non-interleaved; continues samples of a single channel
     * followed by continues samples of the next channel.
     * 
     * The samples are aligned to `samples_per_vector * sizeof(float)` bytes.
     * 
     * Samples is empty when `silent` or `corrupt`.
     */
    std::span<float> samples;

    /** Sample data for a channel.
     * The samples are aligned to `samples_per_vector * sizeof(float)` bytes.
     * 
     * @return The samples for the selected channel. Or empty when this block
     *         is `silent` or `corrupt`.
     */
    [[nodiscard]] std::span<float> samples_for_channel(ssize_t index) noexcept
    {
        tt_axiom(index >= 0 && index < number_of_channels);
        if (samples.empty()) {
            return {};
        } else {
            tt_axiom(ssize(samples) == number_of_channels * number_of_samples());
            return samples.subspan(index * number_of_samples(), number_of_samples());
        }
    }

    /** The sample position of the first sample in this block
     * since the start of the capture/render session.
     */
    uint64_t sample_position;

    /** Timestamp when the sample first sample in this block
     * was captured at the audio device input or when the first sample
     * in this block will appear at the audio device output.
     * 
     * hires_utc_clock::time_point::max() when the timestamp is invalid.
     */
    hires_utc_clock::time_point timestamp;

    /** Sample rate of the word clock attached to the audio interface
     * For example in some situation on film sets the audio interface
     * sample rate is overdriven to 48048 Hz.
     */
    double word_clock_sample_rate;

    /** Sample rate to what the audio device is configured to.
     */
    double device_sample_rate;

    /** The sample data in this block was corrupted
     * Examples of how corruption could happen:
     *  - CRC error caused by a bad USB/Firewire/Ethernet cable.
     *  - Sample rate of the word clock or digital audio input
     *    and the sample rate of the audio device are too far off.
     * 
     * If true data is nullptr.
     */
    bool corrupt;

    /** This block of audio is silent.
     *
     * If true data is nullptr.
     */
    bool silent;
};


}