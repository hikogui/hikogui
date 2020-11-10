// Copyright 2019 Pokitec
// All rights reserved.

#include "audio_system_win32.hpp"
#include "audio_device_win32.hpp"
#include "../required.hpp"
#include "../logger.hpp"
#include "../exceptions.hpp"
#include <Windows.h>
#include <mmdeviceapi.h>

namespace tt {

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

audio_system_win32::audio_system_win32(audio_system_delegate *delegate) :
    audio_system(delegate)
{
    hresult_assert_or_throw(CoInitializeEx(NULL, COINIT_MULTITHREADED));

    hresult_assert_or_throw(CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        &_device_enumerator
    ));
    tt_assert(_device_enumerator != nullptr);
}

audio_system_win32::~audio_system_win32()
{
    auto deviceEnumerator_ = static_cast<IMMDeviceEnumerator *>(_device_enumerator);
        
    tt_assert(deviceEnumerator_ != nullptr);
    deviceEnumerator_->Release();
}

void audio_system_win32::initialize() noexcept
{
    ttlet lock = std::scoped_lock(audio_system::mutex);

    audio_system::initialize();
    update_device_list();
    _delegate->audio_device_list_changed();
}

void audio_system_win32::update_device_list() noexcept
{
    ttlet lock = std::scoped_lock(audio_system::mutex);

    tt_assert(_device_enumerator != nullptr);
    auto device_enumerator = static_cast<IMMDeviceEnumerator *>(_device_enumerator);

    IMMDeviceCollection *device_collection;
    hresult_assert_or_throw(device_enumerator->EnumAudioEndpoints(
        eAll,
        DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED | DEVICE_STATE_NOTPRESENT | DEVICE_STATE_UNPLUGGED,
        &device_collection
    ));
    tt_assert(device_collection != nullptr);

    UINT number_of_devices;
    hresult_assert_or_throw(device_collection->GetCount(&number_of_devices));

    auto old_devices = _devices;
    _devices.clear();
    for (UINT i = 0; i < number_of_devices; i++) {
        IMMDevice *win32_device;
        hresult_assert_or_throw(device_collection->Item(i, &win32_device));

        ttlet device_id = audio_device_win32::get_id_from_device(win32_device);

        auto it = std::find_if(old_devices.begin(), old_devices.end(), [&device_id](auto &item) {
            return item->id == device_id;
        });

        if (it != old_devices.end()) {
            // This device was already instantiated.
            win32_device->Release();
            _devices.push_back(std::move(*it));
            old_devices.erase(it);

        } else {
            auto device = std::make_shared<audio_device_win32>(win32_device);
            LOG_INFO("Found audio device {} state={}", device->name(), device->state());
            _devices.push_back(std::move(device));
        }
    }

    device_collection->Release();

    // Any devices in old_devices that are left over will be deallocated.
}

}