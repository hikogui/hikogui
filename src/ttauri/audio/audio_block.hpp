// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../hires_utc_clock.hpp"
#include "../required.hpp"
#include "../assert.hpp"
#include "../geometry/numeric_array.hpp"
#include <span>

namespace tt {

enum class audio_block_state { normal, silent, corrupt };

/** A block of audio data.
 * This represents a block of audio data received from, or to be send to,
 * an audio device.
 *
 * The samples in this block are always in native floating point format for
 * easy processing. The samples are stored continues for each channel so that
 * processing can be done at a per-channel basis using SSE instructions.
 */
class audio_block {
public:
    /** A list of pointers to non-interleaved sample buffers.
     * For both recording and playback it is allowed to modify the samples
     * in the buffers.
     *
     * The sample buffers are NOT pre-cleared during recording.
     */
    std::vector<float *> sample_buffers;

    /** Number of samples for each channel in sample_buffers.
     */
    size_t num_samples;

    /** The sample rate this block was taken at.
     * This is the word-clock rate, not the sample rate the device was configured as.
     */
    int sample_rate;

    /** The sample count value for the first sample in the sample buffers.
     */
    int64_t sample_count;

    /** Time point when the sample was at the input or will be at the output of the audio interface.
     */
    hires_utc_clock::time_point time_stamp;

    /** The state of the audio block.
     *  - normal: The sample buffers contain normalized -1.0 to 1.0 sample data.
     *  - silent: The sample buffers contain 0.0.
     *  - corrupt: The sample buffers contain NaN.
     *
     * Examples of how corruption could happen:
     *  - CRC error caused by a bad USB/Firewire/Ethernet cable.
     *  - Sample rate of the word clock or digital audio input
     *    and the sample rate of the audio device are too far off.
     *
     * When the state is corrupt; DO NOT READ THE SAMPLE_BUFFER.
     */
    audio_block_state state;
};

} // namespace tt