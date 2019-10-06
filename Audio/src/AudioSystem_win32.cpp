// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Audio/AudioSystem_win32.hpp"
#include "TTauri/Audio/AudioDevice_win32.hpp"
#include "TTauri/Required/required.hpp"
#include "TTauri/Diagnostic/logger.hpp"
#include <Windows.h>
#include <mmdeviceapi.h>

namespace TTauri::Audio {

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

AudioSystem_win32::AudioSystem_win32(std::shared_ptr<AudioSystemDelegate> delegate) :
    AudioSystem(delegate)
{
    hresult_assert(CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        &deviceEnumerator
    ));
    required_assert(deviceEnumerator != nullptr);
    

}

AudioSystem_win32::~AudioSystem_win32()
{
    auto deviceEnumerator_ = static_cast<IMMDeviceEnumerator *>(deviceEnumerator);
        
    required_assert(deviceEnumerator_ != nullptr);
    deviceEnumerator_->Release();
}

void AudioSystem_win32::updateDeviceList() noexcept
{
    auto deviceEnumerator_ = static_cast<IMMDeviceEnumerator *>(deviceEnumerator);

    IMMDeviceCollection *deviceCollection;
    required_assert(deviceEnumerator_ != nullptr);
    hresult_assert(deviceEnumerator_->EnumAudioEndpoints(
        eAll,
        DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED | DEVICE_STATE_NOTPRESENT | DEVICE_STATE_UNPLUGGED,
        &deviceCollection
    ));

    UINT numberOfDevices;
    required_assert(deviceCollection != nullptr);
    hresult_assert(deviceCollection->GetCount(&numberOfDevices));

    for (UINT i = 0; i < numberOfDevices; i++) {
        IMMDevice *device;
        hresult_assert(deviceCollection->Item(i, &device));

        let device_id = AudioDevice_win32::getIdFromDevice(device);
        if (hasDeviceWithId(device_id)) {
            device->Release();
        } else {
            auto audioDevice = std::make_unique<AudioDevice_win32>(device);
            LOG_INFO("Found audio device {}", audioDevice->name());
            devices.push_back(std::move(audioDevice));
        }
    }

    deviceCollection->Release();
}

}