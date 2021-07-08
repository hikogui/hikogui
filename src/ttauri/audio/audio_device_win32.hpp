// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_device.hpp"

struct IMMDevice;
struct IPropertyStore;
struct IMMEndpoint;
struct IAudioEndpointVolume;
struct IAudioMeterInformation;

namespace tt {

/*! A class representing an audio device on the system.
*/
class audio_device_win32 : public audio_device {
public:
    audio_device_win32(IMMDevice *device);
    ~audio_device_win32();

    [[nodiscard]] std::string id() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override;
    [[nodiscard]] tt::label label() const noexcept override;
    [[nodiscard]] audio_device_state state() const noexcept override;
    [[nodiscard]] audio_direction direction() const noexcept override;
    [[nodiscard]] size_t full_num_channels() const noexcept override;
    [[nodiscard]] speaker_mapping full_channel_mapping() const noexcept override;

    static std::string get_id_from_device(IMMDevice *device) noexcept;

private:
    IMMDevice *_device;
    IMMEndpoint *_end_point;
    IPropertyStore *_property_store;
    IAudioEndpointVolume *_end_point_volume;
    IAudioMeterInformation *_audio_meter_information;

    /** Get a user friendly name of the audio device.
     * This is the name of the audio device itself, such as
     * "Realtek High Definition Audio".
     */
    std::string device_name() const noexcept;

    /** Get a user friendly name of the audio end-point device.
     * This is the name of the end point, such as "Microphone".
     */
    std::string end_point_name() const noexcept;
};

}