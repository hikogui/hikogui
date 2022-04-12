// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../bigint.hpp"
#include "audio_device_delegate.hpp"
#include "audio_stream_config.hpp"
#include "audio_channel.hpp"
#include "audio_direction.hpp"
#include "speaker_mapping.hpp"
#include "audio_device_id.hpp"
#include "../label.hpp"
#include "../enum_metadata.hpp"
#include <string>
#include <memory>
#include <ostream>

namespace tt::inline v1 {

enum class audio_device_state { active, disabled, not_present, unplugged };

// clang-format off
constexpr auto audio_device_state_metadata = enum_metadata{
    audio_device_state::active, "active",
    audio_device_state::disabled, "disabled",
    audio_device_state::not_present, "not_present",
    audio_device_state::unplugged, "unplugged",
};
// clang-format on


[[nodiscard]] inline std::string_view to_string(audio_device_state const &rhs) noexcept
{
    return audio_device_state_metadata[rhs];
}

inline std::ostream &operator<<(std::ostream &lhs, audio_device_state const &rhs)
{
    return lhs << audio_device_state_metadata[rhs];
}

/** A set of audio channels which can be rendered and/or captures at the same time.
 * On win32 this would be Audio Endpoint gfx_device, which can either render or capture
 * but not at the same time.
 *
 * On MacOS this would contain all the inputs and outputs of either a physical or
 * aggregate device that can run in the same clock domain, with both render and
 * capture at the same time.
 */
class audio_device {
public:
    audio_device() noexcept = default;
    virtual ~audio_device() = default;

    /** The nonephemeral unique id that for an audio device on the system.
     */
    audio_device_id id;

    /** Get a user friendly name of the audio device.
     * This is a combination of the name of the device and
     * the name of the end-point.
     */
    [[nodiscard]] virtual std::string name() const noexcept = 0;

    /** Get a user friendly label of the audio device.
     * This is a combination of the name of the device and
     * the name of the end-point, plus an icon for the driver architecture.
     */
    [[nodiscard]] virtual label label() const noexcept = 0;

    /** Get the current state of the audio device.
     */
    [[nodiscard]] virtual audio_device_state state() const noexcept = 0;

    [[nodiscard]] virtual audio_direction direction() const noexcept = 0;

    /** Check if the device is in exclusive mode.
     *
     * @return True to if exclusive mode, False if shared mode
     */
    [[nodiscard]] virtual bool exclusive() const noexcept = 0;

    /** Set the device in exclusive or shared mode.
     *
     * In shared mode:
     *  - The sample rate is the same as the operating system's mixer.
     *  - The speaker mapping is the same as the operating system's mixer.
     *
     * In exclusive mode:
     *  - The sample rate can be changed, and the physical audio device will be
     *    configured to it.
     *  - The speaker mapping can be changed, and the physical device will
     *    configure its input and outputs accordingly.
     *
     * @param exclusive True to get exclusive mode, False to get shared mode.
     */
    virtual void set_exclusive(bool exclusive) noexcept = 0;

    /** Get the currently configured sample rate.
     *
     * @return The current sample rate, or 0.0f when the sample rate is not
     * configured.
     */
    [[nodiscard]] virtual double sample_rate() const noexcept = 0;

    /** Set the sample rate.
     * @param sample_rate The sample rate to configure the device to.
     */
    virtual void set_sample_rate(double sample_rate) noexcept = 0;

    /** Get the currently configured input speaker mapping.
     *
     * @return The current configured input speaker mapping.
     */
    [[nodiscard]] virtual tt::speaker_mapping input_speaker_mapping() const noexcept = 0;

    /** Set the input speaker mapping.
     *
     * @param speaker_mapping The input speaker mapping to configure the device to.
     */
    virtual void set_input_speaker_mapping(tt::speaker_mapping speaker_mapping) noexcept = 0;

    /** Speaker mapping that are available in the current configuration.
     *
     * @return A list of speaker mappings.
     */
    [[nodiscard]] virtual std::vector<tt::speaker_mapping> available_input_speaker_mappings() const noexcept = 0;

    /** Get the currently configured output speaker mapping.
     *
     * @return The current configured output speaker mapping.
     */
    [[nodiscard]] virtual tt::speaker_mapping output_speaker_mapping() const noexcept = 0;

    /** Set the output speaker mapping.
     *
     * @param speaker_mapping The output speaker mapping to configure the device to.
     */
    virtual void set_output_speaker_mapping(tt::speaker_mapping speaker_mapping) noexcept = 0;

    /** Speaker mapping that are available in the current configuration.
     *
     * @return A list of speaker mappings.
     */
    [[nodiscard]] virtual std::vector<tt::speaker_mapping> available_output_speaker_mappings() const noexcept = 0;

    /** Start a session. Start a session, which will cause data to be stream to
     * and from the audio device and the delegate's process_audio() function to
     * be called.
     *
     * This function may spawn a thread to handle the audio processing. This
     * function may throw an exception if it is not possible to start a session.
     *
     * @param id a unique ID used by the operating system to remember audio
     *        parameters for this stream, such as volume, across reboots.
     * @param name A name used to by the operating system to display to the user
     *        when changing audio parameters through the operating system's
     *        preferences.
     * bit-depth. XXX Windows allows for an icon to be passed to a session.
     */
    // virtual void start_stream(std::string id, std::string name) = 0;

    /** Stop a session.
     * Stop a session, which will also stop the streams of audio.
     */
    // virtual void stop_stream() noexcept = 0;

private:
    std::shared_ptr<audio_device_delegate> delegate = {};
};

} // namespace tt::inline v1


template<typename CharT>
struct std::formatter<tt::audio_device_state, CharT> : std::formatter<char const *, CharT> {
    auto format(tt::audio_device_state const &t, auto &fc)
    {
        return std::formatter<char const *, CharT>::format(tt::to_const_string(t), fc);
    }
};
