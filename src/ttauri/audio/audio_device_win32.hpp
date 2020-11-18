// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "audio_device.hpp"

struct IMMDevice;
struct IPropertyStore;
struct IMMEndpoint;

namespace tt {

/*! A class representing an audio device on the system.
*/
class audio_device_win32 : public audio_device {
public:
    audio_device_win32(IMMDevice *device);
    ~audio_device_win32();

    std::string id() const noexcept override;
    std::string name() const noexcept override;
    tt::label label() const noexcept override;
    audio_device_state state() const noexcept override;
    audio_device_flow_direction direction() const noexcept override;

    static std::string get_id_from_device(IMMDevice *device) noexcept;

private:
    IMMDevice *_device;
    IMMEndpoint *_endpoint;
    IPropertyStore *_property_store;

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