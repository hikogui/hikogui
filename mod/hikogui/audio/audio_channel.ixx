// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <cstddef>
#include <string>

export module hikogui_audio_audio_channel;
import hikogui_audio_audio_direction;

export namespace hi { inline namespace v1 {

export class audio_channel {
public:
    /** If the channel is enabled by the user.
     *
     * The following situations exist.
     *  - input, enabled: Audio from device is available in the input buffer of the audio proc.
     *  - input, disabled: The input buffer is made silent, the peak and rms values are zero to.
     *  - output, enabled: Audio from the output buffer is copied to the audio device.
     *  - output, disabled: Silence is send to the audio device. The peak and rms values are zero.
     */
    bool enabled;

    /** The index of the audio channel.
     * The index is per direction. There is a index zero for both input and output direction.
     */
    [[nodiscard]] std::size_t index() const noexcept;

    /** The direction of audio.
     */
    [[nodiscard]] audio_direction direction() const noexcept;

    /** A permanent id for the channel for this device.
     */
    [[nodiscard]] std::string id() const noexcept;

    /** The name of the audio channel according to the system.
     */
    [[nodiscard]] std::string name() const noexcept;

    /** The number of times the audio clipped since last read.
     */
    [[nodiscard]] std::size_t clip_count() noexcept;

    /** The maximum peak sample value since last read.
     */
    [[nodiscard]] float peak() noexcept;

    /** Get the running-RMS over the given number of samples.
     * Granularity of the samples is the number of samples of an audio proc.
     *
     * @param num_samples Number of samples to take the running average over.
     */
    [[nodiscard]] float rms(std::size_t num_samples) noexcept;
};

}} // namespace hi::inline v1
