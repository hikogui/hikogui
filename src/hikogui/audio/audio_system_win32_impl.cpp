// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../utility/win32_headers.hpp"

#include "audio_system_win32.hpp"
#include "audio_device_win32.hpp"
#include "audio_system_aggregate.hpp"
#include "../utility/module.hpp"
#include "../log.hpp"
#include "../locked_memory_allocator.hpp"
#include "../loop.hpp"

hi_warning_push();
// C26435: Function '' should specify exactly one of 'virtual', 'override' or 'final' (c.128)
// STDMETHOD macro from the win32 API includes virtual.
hi_warning_ignore_msvc(26435);

namespace hi { inline namespace v1 {

[[nodiscard]] std::unique_ptr<audio_system> audio_system::make_unique() noexcept
{
    auto tmp = std::make_unique<audio_system_aggregate>();
    tmp->add_child(std::make_unique<audio_system_win32>());
    // Possibly add asio here as well.
    return tmp;
}

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

class audio_system_win32_notification_client : public IMMNotificationClient {
public:
    virtual ~audio_system_win32_notification_client() = default;

    audio_system_win32_notification_client(audio_system_win32 *system) : IMMNotificationClient(), _system(system) {}

    STDMETHOD(OnDefaultDeviceChanged)(EDataFlow flow, ERole role, LPCWSTR device_id) override
    {
        loop::main().wfree_post_function([this]() {
            _system->update_device_list();
            _system->_notifier();
        });
        return S_OK;
    }

    STDMETHOD(OnDeviceAdded)(LPCWSTR device_id) override
    {
        loop::main().wfree_post_function([this]() {
            _system->update_device_list();
            _system->_notifier();
        });
        return S_OK;
    }

    STDMETHOD(OnDeviceRemoved)(LPCWSTR device_id) override
    {
        // OnDeviceRemoved can not be implemented according to the win32 specification due
        // to conflicting requirements.
        // 1. This function may not block.
        // 2. The string pointed by device_id will not exist after this function.
        // 2. We need to copy device_id which has an unbounded size.
        // 3. Allocating a string blocks.

        hi_assert_not_null(device_id);
        loop::main().wfree_post_function([this]() {
            _system->update_device_list();
            _system->_notifier();
        });
        return S_OK;
    }

    STDMETHOD(OnDeviceStateChanged)(LPCWSTR device_id, DWORD state) override
    {
        loop::main().wfree_post_function([this]() {
            _system->update_device_list();
            _system->_notifier();
        });
        return S_OK;
    }

    STDMETHOD(OnPropertyValueChanged)(LPCWSTR device_id, PROPERTYKEY const key) override
    {
        loop::main().wfree_post_function([this]() {
            _system->update_device_list();
            _system->_notifier();
        });
        return S_OK;
    }

    STDMETHOD(QueryInterface)(REFIID iid, void **object) override
    {
        hi_no_default();
    }

    STDMETHOD_(ULONG, AddRef)() override
    {
        hi_no_default();
    }

    STDMETHOD_(ULONG, Release)() override
    {
        hi_no_default();
    }

private:
    audio_system_win32 *_system;
};

audio_system_win32::audio_system_win32() :
    super(), _notification_client(std::make_unique<audio_system_win32_notification_client>(this))
{
    hi_hresult_check(CoInitializeEx(NULL, COINIT_MULTITHREADED));

    hi_hresult_check(CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, reinterpret_cast<LPVOID *>(&_device_enumerator)));
    hi_assert(_device_enumerator);

    _device_enumerator->RegisterEndpointNotificationCallback(_notification_client.get());

    // Start with enumerating the devices.
    update_device_list();
}

audio_system_win32::~audio_system_win32()
{
    if (_device_enumerator) {
        _device_enumerator->UnregisterEndpointNotificationCallback(_notification_client.get());
        _device_enumerator->Release();
    }
}

void audio_system_win32::update_device_list() noexcept
{
    hi_axiom(loop::main().on_thread());
    hi_log_info("Updating audio device list:");

    IMMDeviceCollection *device_collection;
    if (FAILED(_device_enumerator->EnumAudioEndpoints(
            eAll, DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED | DEVICE_STATE_UNPLUGGED, &device_collection))) {
        hi_log_error("EnumAudioEndpoints() failed: {}", get_last_error_message());
        return;
    }
    hi_assert(device_collection);

    UINT number_of_devices;
    if (FAILED(device_collection->GetCount(&number_of_devices))) {
        hi_log_error("EnumAudioEndpoints()->GetCount() failed: {}", get_last_error_message());
        device_collection->Release();
        return;
    }

    auto old_devices = _devices;
    _devices.clear();
    for (UINT i = 0; i < number_of_devices; i++) {
        IMMDevice *win32_device;
        if (FAILED(device_collection->Item(i, &win32_device))) {
            hi_log_error("EnumAudioEndpoints()->Item({}) failed: {}", i, get_last_error_message());
            device_collection->Release();
            return;
        }
        hi_assert(win32_device);

        auto win32_device_id = std::string{};
        try {
            win32_device_id = std::string{"win32:"} + audio_device_win32::get_device_id(win32_device);
        } catch (std::exception const& e) {
            hi_log_error("EnumAudioEndpoints()->Item({})->get_device_id failed: {}", i, e.what());
            device_collection->Release();
            win32_device->Release();
            return;
        }

        auto it = std::find_if(old_devices.begin(), old_devices.end(), [&win32_device_id](auto& item) {
            return item->id() == win32_device_id;
        });

        if (it != old_devices.end()) {
            // This device was already instantiated.
            win32_device->Release();
            _devices.push_back(std::move(*it));
            old_devices.erase(it);

            // Let the audio device them self see if anything has changed in their own state.
            _devices.back()->update_state();

        } else {
            auto device = std::allocate_shared<audio_device_win32>(locked_memory_allocator<audio_device_win32>{}, win32_device);
            // hi_log_info(
            //    "Found audio device \"{}\", state={}, channels={}, speakers={}",
            //    device->name(),
            //    device->state(),
            //    device->full_num_channels(),
            //    device->full_channel_mapping());
            _devices.push_back(std::move(device));
        }
    }

    device_collection->Release();
}

}} // namespace hi::v1

hi_warning_pop();
