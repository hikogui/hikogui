// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../win32_headers.hpp"

#include "win32_device_interface.hpp"
#include "../log.hpp"
#include "../exception.hpp"

namespace hi::inline v1 {

// static auto KSPROPSETID_Pin_GUID = GUID{KSPROPSETID_Pin};

win32_device_interface::win32_device_interface(std::string device_name) :
    _device_name(std::move(device_name)), _handle(INVALID_HANDLE_VALUE)
{
    auto device_name_ = hi::to_wstring(_device_name);

    _handle = CreateFileW(device_name_.c_str(), FILE_SHARE_READ | FILE_SHARE_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (_handle == INVALID_HANDLE_VALUE) {
        throw io_error(std::format("Could not open win32_device_interface {}: {}", _device_name, get_last_error_message()));
    }
}

win32_device_interface::~win32_device_interface()
{
    if (_handle != INVALID_HANDLE_VALUE) {
        if (not CloseHandle(_handle)) {
            hi_log_error("Could not close win32_device_interface {}: {}", _device_name, get_last_error_message());
        }
    }
}

[[nodiscard]] std::string win32_device_interface::pin_name(ULONG pin_nr) const noexcept
{
    try {
        return get_pin_property<std::string>(pin_nr, KSPROPERTY_PIN_NAME);
    } catch (std::exception const& e) {
        hi_log_error("Could not get pin-name on device {}: {}", _device_name, e.what());
        return std::string{"<unknown pin>"};
    }
}

[[nodiscard]] ULONG win32_device_interface::pin_count() const noexcept
{
    try {
        return wide_cast<ULONG>(get_pin_property<DWORD>(0, KSPROPERTY_PIN_CTYPES));
    } catch (std::exception const& e) {
        hi_log_error("Could not get pin-count on device {}: {}", _device_name, e.what());
        return 0;
    }
}

[[nodiscard]] bool win32_device_interface::is_streaming_interface(ULONG pin_nr) const noexcept
{
    try {
        for (auto *identifier : get_pin_properties<KSIDENTIFIER>(pin_nr, KSPROPERTY_PIN_INTERFACES)) {
            if (not IsEqualGUID(identifier->Set, KSINTERFACESETID_Standard)) {
                continue;
            }
            if (identifier->Id == KSINTERFACE_STANDARD_STREAMING or identifier->Id == KSINTERFACE_STANDARD_LOOPED_STREAMING) {
                return true;
            }
        }
    } catch (std::exception const& e) {
        hi_log_error("Could not get pin-interface property for {} pin_nr {}: {}", _device_name, pin_nr, e.what());
    }
    return false;
}

[[nodiscard]] bool win32_device_interface::is_standerdio_medium(ULONG pin_nr) const noexcept
{
    try {
        for (hilet& identifier : get_pin_properties<KSIDENTIFIER>(pin_nr, KSPROPERTY_PIN_MEDIUMS)) {
            if (not IsEqualGUID(identifier->Set, KSMEDIUMSETID_Standard)) {
                continue;
            }
            if (identifier->Id == KSMEDIUM_STANDARD_DEVIO) {
                return true;
            }
        }
    } catch (std::exception const& e) {
        hi_log_error("Could not get pin-medium property for {} pin_nr {}: {}", _device_name, pin_nr, e.what());
    }
    return false;
}

[[nodiscard]] bool win32_device_interface::is_streaming_pin(ULONG pin_nr, audio_direction direction) const noexcept
{
    // Check if this is a streaming-pin.
    if (not is_streaming_interface(pin_nr)) {
        return false;
    }

    if (not is_standerdio_medium(pin_nr)) {
        return false;
    }

    // Check if the dataflow direction of the pin is in the opposite-direction of the end-point.
    hilet flow = get_pin_property<KSPIN_DATAFLOW>(pin_nr, KSPROPERTY_PIN_DATAFLOW);
    if (direction == audio_direction::input and flow != KSPIN_DATAFLOW_OUT) {
        return false;
    } else if (direction == audio_direction::output and flow != KSPIN_DATAFLOW_IN) {
        return false;
    }

    // Check if the communication channel direction is correct for a streaming-pin.
    //hilet communication = pin_communication(pin_nr);
    //if (communication != KSPIN_COMMUNICATION_SINK and communication != KSPIN_COMMUNICATION_BOTH) {
    //    return false;
    //}

    return true;
}

[[nodiscard]] generator<ULONG> win32_device_interface::find_streaming_pins(audio_direction direction)
{
    hi_axiom(direction != audio_direction::bidirectional and direction != audio_direction::none);

    hilet num_pins = pin_count();
    for (ULONG pin_nr = 0; pin_nr != num_pins; ++pin_nr) {
        if (is_streaming_pin(pin_nr, direction)) {
            co_yield pin_nr;
        }
    }
}

[[nodiscard]] GUID win32_device_interface::pin_category(ULONG pin_nr) const noexcept
{
    try {
        return get_pin_property<GUID>(0, KSPROPERTY_PIN_CATEGORY);
    } catch (std::exception const& e) {
        hi_log_error("Could not get pin-category on device {}: {}", _device_name, e.what());
        return {};
    }
}

[[nodiscard]] KSPIN_COMMUNICATION win32_device_interface::pin_communication(ULONG pin_nr) const noexcept
{
    try {
        return get_pin_property<KSPIN_COMMUNICATION>(0, KSPROPERTY_PIN_COMMUNICATION);
    } catch (std::exception const& e) {
        hi_log_error("Could not get pin-communication on device {}: {}", _device_name, e.what());
        return KSPIN_COMMUNICATION_NONE;
    }
}


} // namespace hi::inline v1