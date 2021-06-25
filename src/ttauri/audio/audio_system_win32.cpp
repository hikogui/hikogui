// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_system_win32.hpp"
#include "audio_device_win32.hpp"
#include "../required.hpp"
#include "../logger.hpp"
#include "../exception.hpp"
#include "../locked_memory_allocator.hpp"
#include <Windows.h>
#include <mmdeviceapi.h>

namespace tt {

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

class audio_system_win32_notification_client : public IMMNotificationClient {
public:
    audio_system_win32_notification_client(audio_system_win32 *system) : IMMNotificationClient(), _system(system) {}

    STDMETHOD(OnDefaultDeviceChanged)(EDataFlow flow, ERole role, LPCWSTR device_id)
    {
        _system->default_device_changed();
        return S_OK;
    }

    STDMETHOD(OnDeviceAdded)(LPCWSTR device_id)
    {
        _system->device_added();
        return S_OK;
    }

    STDMETHOD(OnDeviceRemoved)(LPCWSTR device_id)
    {
        auto device_id_ = device_id == nullptr ? "win32:"s : "win32:"s + to_string(std::wstring(device_id));
        _system->device_removed(device_id_);
        return S_OK;
    }

    STDMETHOD(OnDeviceStateChanged)(LPCWSTR device_id, DWORD state)
    {
        auto device_id_ = device_id == nullptr ? "win32:"s : "win32:"s + to_string(std::wstring(device_id));
        _system->device_state_changed(device_id_);
        return S_OK;
    }

    STDMETHOD(OnPropertyValueChanged)(LPCWSTR device_id, PROPERTYKEY const key)
    {
        auto device_id_ = device_id == nullptr ? "win32:"s : "win32:"s + to_string(std::wstring(device_id));
        _system->device_property_value_changed(device_id_);
        return S_OK;
    }

    STDMETHOD(QueryInterface)(REFIID iid, void **object)
    {
        tt_no_default();
    }

    STDMETHOD_(ULONG, AddRef)()
    {
        tt_no_default();
    }

    STDMETHOD_(ULONG, Release)()
    {
        tt_no_default();
    }

private:
    audio_system_win32 *_system;
};

audio_system_win32::audio_system_win32(weak_or_unique_ptr<audio_system_delegate> delegate) : audio_system(std::move(delegate))
{
    tt_hresult_check(CoInitializeEx(NULL, COINIT_MULTITHREADED));

    tt_hresult_check(CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, reinterpret_cast<LPVOID *>(&_device_enumerator)));
    tt_assert(_device_enumerator != nullptr);

    _notification_client = new audio_system_win32_notification_client(this);

    _device_enumerator->RegisterEndpointNotificationCallback(_notification_client);
}

audio_system_win32::~audio_system_win32()
{
    _device_enumerator->UnregisterEndpointNotificationCallback(_notification_client);
    delete _notification_client;
    _device_enumerator->Release();
}

void audio_system_win32::init() noexcept
{
    {
        ttlet lock = std::scoped_lock(audio_system::mutex);
        audio_system::init();
        update_device_list();
    }
    if (auto delegate = delegate_lock()) {
        delegate->audio_device_list_changed(*this);
    }
}

void audio_system_win32::update_device_list() noexcept
{
    ttlet lock = std::scoped_lock(audio_system::mutex);

    IMMDeviceCollection *device_collection;
    tt_hresult_check(_device_enumerator->EnumAudioEndpoints(
        eAll, DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED | DEVICE_STATE_UNPLUGGED, &device_collection));
    tt_assert(device_collection != nullptr);

    UINT number_of_devices;
    tt_hresult_check(device_collection->GetCount(&number_of_devices));

    auto old_devices = _devices;
    _devices.clear();
    for (UINT i = 0; i < number_of_devices; i++) {
        IMMDevice *win32_device;
        tt_hresult_check(device_collection->Item(i, &win32_device));

        ttlet device_id = audio_device_win32::get_id_from_device(win32_device);

        auto it = std::find_if(old_devices.begin(), old_devices.end(), [&device_id](auto &item) {
            return item->id() == device_id;
        });

        if (it != old_devices.end()) {
            // This device was already instantiated.
            win32_device->Release();
            _devices.push_back(std::move(*it));
            old_devices.erase(it);

        } else {
            auto device = std::allocate_shared<audio_device_win32>(locked_memory_allocator<audio_device_win32>{}, win32_device);
            // tt_log_info(
            //    "Found audio device \"{}\", state={}, channels={}, speakers={}",
            //    device->name(),
            //    device->state(),
            //    device->full_num_channels(),
            //    device->full_channel_mapping());
            _devices.push_back(std::move(device));
        }
    }

    device_collection->Release();

    // Any devices in old_devices that are left over will be deallocated.
}

void audio_system_win32::default_device_changed() noexcept {}

void audio_system_win32::device_added() noexcept
{
    update_device_list();
    if (auto delegate = _delegate.lock()) {
        delegate->audio_device_list_changed(*this);
    }
}

void audio_system_win32::device_removed(std::string device_id) noexcept
{
    update_device_list();
    if (auto delegate = _delegate.lock()) {
        delegate->audio_device_list_changed(*this);
    }
}

void audio_system_win32::device_state_changed(std::string device_id) noexcept
{
    if (auto delegate = _delegate.lock()) {
        delegate->audio_device_list_changed(*this);
    }
}

void audio_system_win32::device_property_value_changed(std::string device_id) noexcept
{
    if (auto delegate = _delegate.lock()) {
        delegate->audio_device_list_changed(*this);
    }
}

} // namespace tt