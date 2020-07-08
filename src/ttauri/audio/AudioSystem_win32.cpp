// Copyright 2019 Pokitec
// All rights reserved.

#include "AudioSystem_win32.hpp"
#include "AudioDevice_win32.hpp"
#include "../required.hpp"
#include "../logger.hpp"
#include "../exceptions.hpp"
#include <Windows.h>
#include <mmdeviceapi.h>

namespace tt {

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

AudioSystem_win32::AudioSystem_win32(AudioSystemDelegate *delegate) :
    AudioSystem(delegate)
{
    hresult_assert_or_throw(CoInitializeEx(NULL, COINIT_MULTITHREADED));

    hresult_assert_or_throw(CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        &deviceEnumerator
    ));
    tt_assert(deviceEnumerator != nullptr);
}

AudioSystem_win32::~AudioSystem_win32()
{
    auto deviceEnumerator_ = static_cast<IMMDeviceEnumerator *>(deviceEnumerator);
        
    tt_assert(deviceEnumerator_ != nullptr);
    deviceEnumerator_->Release();
}

void AudioSystem_win32::initialize() noexcept
{
    AudioSystem::initialize();
    updateDeviceList();
    delegate->audioDeviceListChanged();
}

void AudioSystem_win32::updateDeviceList() noexcept
{
    auto deviceEnumerator_ = static_cast<IMMDeviceEnumerator *>(deviceEnumerator);

    IMMDeviceCollection *deviceCollection;
    tt_assert(deviceEnumerator_ != nullptr);
    hresult_assert_or_throw(deviceEnumerator_->EnumAudioEndpoints(
        eAll,
        DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED | DEVICE_STATE_NOTPRESENT | DEVICE_STATE_UNPLUGGED,
        &deviceCollection
    ));

    UINT numberOfDevices;
    tt_assert(deviceCollection != nullptr);
    hresult_assert_or_throw(deviceCollection->GetCount(&numberOfDevices));

    for (UINT i = 0; i < numberOfDevices; i++) {
        IMMDevice *device;
        hresult_assert_or_throw(deviceCollection->Item(i, &device));

        ttlet device_id = AudioDevice_win32::getIdFromDevice(device);
        if (hasDeviceWithId(device_id)) {
            device->Release();
        } else {
            auto audioDevice = std::make_unique<AudioDevice_win32>(device);
            LOG_INFO("Found audio device {} state={}", audioDevice->name(), audioDevice->state());
            devices.push_back(std::move(audioDevice));
        }
    }

    deviceCollection->Release();
}

}