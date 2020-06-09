// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Audio/AudioDevice.hpp"

namespace TTauri {

/*! A class representing an audio device on the system.
*/
class AudioDevice_win32 : public AudioDevice {
private:
    void *device;
    void *propertyStore;

public:
    AudioDevice_win32(void *device);
    ~AudioDevice_win32();

    std::string name() const noexcept override;
    std::string deviceName() const noexcept override;
    std::string endPointName() const noexcept override;
    AudioDevice_state state() const noexcept override;

    static std::string getIdFromDevice(void *device) noexcept;
};

}