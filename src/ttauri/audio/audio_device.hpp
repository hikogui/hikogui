// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../bigint.hpp"
#include "audio_device_delegate.hpp"
#include "audio_stream_config.hpp"
#include "audio_channel.hpp"
#include "audio_direction.hpp"
#include "audio_channel_mapping.hpp"
#include "../label.hpp"
#include <string>
#include <memory>
#include <ostream>

namespace tt {

enum class audio_device_state { active, disabled, not_present, unplugged };

[[nodiscard]] constexpr char const *to_const_string(audio_device_state const &rhs) noexcept
{
    switch (rhs) {
    case audio_device_state::active: return "active";
    case audio_device_state::disabled: return "disabled";
    case audio_device_state::not_present: return "not_present";
    case audio_device_state::unplugged: return "unplugged";
    default: tt_no_default();
    }
}

[[nodiscard]] inline std::string to_string(audio_device_state const &rhs) noexcept
{
    return {to_const_string(rhs)};
}

inline std::ostream &operator<<(std::ostream &lhs, audio_device_state const &rhs)
{
    return lhs << to_const_string(rhs);
}

/** A set of audio channels which can be rendered and/or captures at the same time.
 * On win32 this would be Audio Endpoint gui_device, which can either render or capture
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
    [[nodiscard]] virtual std::string id() const noexcept = 0;

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

    [[nodiscard]] virtual size_t full_num_channels() const noexcept = 0;

    /** Get a bitmap of channel mappings for this audio device.
     * The value returned is independent on the how many channels are actually configured.
     * 
     * @return Mapping of audio channels to speaker locations.
     */
    [[nodiscard]] virtual audio_channel_mapping full_channel_mapping() const noexcept = 0;

    //[[nodiscard]] virtual std::vector<audio_channel> inputs() noexcept;

    //[[nodiscard]] virtual std::vector<audio_channel> outputs() noexcept;

    /** Check if a audio configuration is supported by this device.
     * @param config Configuration such as sample rate, sample format and bit-depth.
     */
    // virtual bool isConfigSupported(AudiostreamConfig config) const noexcept = 0;

    /** Start a session.
     * Start a session, which will cause data to be stream to and
     * from the audio device and the delegate's process_audio() function
     * to be called.
     *
     * This function may spawn a thread to handle the audio processing.
     * This function may throw an exception if it is not possible to start
     * a session.
     *
     * @param id a unique ID used by the operating system to remember
     *        audio parameters for this stream, such as volume, accross reboots.
     * @param name A name used to by the operating system to display to the user
     *        when changing audio parameters through the operating system's preferences.
     * @param config Configuration such as sample rate, sample format and bit-depth.
     * XXX Windows allows for an icon to be passed to a session.
     */
    // virtual void start_stream(std::string id, std::string name, audio_stream_config config) = 0;

    /** Stop a session.
     * Stop a session, which will also stop the streams of audio.
     */
    // virtual void stop_stream() noexcept = 0;

private:
    std::shared_ptr<audio_device_delegate> delegate = {};
};

template<typename CharT>
struct std::formatter<tt::audio_device_state, CharT> : std::formatter<char const *, CharT> {
    auto format(tt::audio_device_state t, auto &fc)
    {
        return std::formatter<char const *, CharT>::format(tt::to_const_string(t), fc);
    }
};

} // namespace tt
