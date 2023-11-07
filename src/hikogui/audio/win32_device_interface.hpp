// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_direction.hpp"
#include "audio_format_range.hpp"
#include "../coroutine/coroutine.hpp"
#include "../utility/utility.hpp"
#include "../telemetry/telemetry.hpp"
#include "../macros.hpp"
#include <string>
#include <coroutine>

hi_export_module(hikogui.audio.win32_device_interface);

hi_export namespace hi { inline namespace v1 {

hi_export class win32_device_interface {
public:
    win32_device_interface(std::string device_name) : _device_name(std::move(device_name)), _handle(INVALID_HANDLE_VALUE)
    {
        auto device_name_ = hi::to_wstring(_device_name);

        _handle = CreateFileW(device_name_.c_str(), FILE_SHARE_READ | FILE_SHARE_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (_handle == INVALID_HANDLE_VALUE) {
            throw io_error(std::format("Could not open win32_device_interface {}: {}", _device_name, get_last_error_message()));
        }
    }

    ~win32_device_interface()
    {
        if (_handle != INVALID_HANDLE_VALUE) {
            if (not CloseHandle(_handle)) {
                hi_log_error("Could not close win32_device_interface {}: {}", _device_name, get_last_error_message());
            }
        }
    }

    [[nodiscard]] ULONG pin_count() const noexcept
    {
        try {
            return wide_cast<ULONG>(get_pin_property<DWORD>(0, KSPROPERTY_PIN_CTYPES));
        } catch (std::exception const& e) {
            hi_log_error("Could not get pin-count on device {}: {}", _device_name, e.what());
            return 0;
        }
    }

    [[nodiscard]] std::string pin_name(ULONG pin_nr) const noexcept
    {
        try {
            return get_pin_property<std::string>(pin_nr, KSPROPERTY_PIN_NAME);
        } catch (std::exception const& e) {
            hi_log_error("Could not get pin-name on device {}: {}", _device_name, e.what());
            return std::string{"<unknown pin>"};
        }
    }

    [[nodiscard]] generator<ULONG> find_streaming_pins(audio_direction direction) const noexcept
    {
        hilet num_pins = pin_count();
        for (ULONG pin_nr = 0; pin_nr != num_pins; ++pin_nr) {
            if (is_streaming_pin(pin_nr, direction)) {
                co_yield pin_nr;
            }
        }
    }

    [[nodiscard]] GUID pin_category(ULONG pin_nr) const noexcept
    {
        try {
            return get_pin_property<GUID>(0, KSPROPERTY_PIN_CATEGORY);
        } catch (std::exception const& e) {
            hi_log_error("Could not get pin-category on device {}: {}", _device_name, e.what());
            return {};
        }
    }

    [[nodiscard]] KSPIN_COMMUNICATION pin_communication(ULONG pin_nr) const noexcept
    {
        try {
            return get_pin_property<KSPIN_COMMUNICATION>(0, KSPROPERTY_PIN_COMMUNICATION);
        } catch (std::exception const& e) {
            hi_log_error("Could not get pin-communication on device {}: {}", _device_name, e.what());
            return KSPIN_COMMUNICATION_NONE;
        }
    }

    [[nodiscard]] generator<audio_format_range> get_format_ranges(ULONG pin_nr) const noexcept
    {
        for (auto *format_range : get_pin_properties<KSDATARANGE>(pin_nr, KSPROPERTY_PIN_DATARANGES)) {
            if (IsEqualGUID(format_range->MajorFormat, KSDATAFORMAT_TYPE_AUDIO)) {
                auto has_int = false;
                auto has_float = false;
                if (IsEqualGUID(format_range->SubFormat, KSDATAFORMAT_SUBTYPE_PCM)) {
                    has_int = true;
                } else if (IsEqualGUID(format_range->SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
                    has_float = true;
                } else if (IsEqualGUID(format_range->SubFormat, KSDATAFORMAT_SUBTYPE_WILDCARD)) {
                    has_int = true;
                    has_float = true;
                } else {
                    // The scarlett returns KSDATAFORMAT_SUBTYPE_ANALOGUE for one of the pins,
                    // we can't filter scarlet for the proper streaming-pin.
                    continue;
                }

                hilet *format_range_ = reinterpret_cast<KSDATARANGE_AUDIO const *>(format_range);
                if (format_range_->MinimumBitsPerSample > 64) {
                    hi_log_error(
                        "Bad KSDATARANGE_AUDIO MinimumBitsPerSample == {} for device {}",
                        format_range_->MinimumBitsPerSample,
                        _device_name);
                    continue;
                }
                if (format_range_->MaximumBitsPerSample > 64) {
                    hi_log_error(
                        "Bad KSDATARANGE_AUDIO MaximumBitsPerSample == {} for device {}",
                        format_range_->MaximumBitsPerSample,
                        _device_name);
                    continue;
                }
                if (format_range_->MinimumBitsPerSample > format_range_->MaximumBitsPerSample) {
                    hi_log_error(
                        "Bad KSDATARANGE_AUDIO MinimumBitsPerSample == {}, MaximumBitsPerSample {} for device {}",
                        format_range_->MinimumBitsPerSample,
                        format_range_->MaximumBitsPerSample,
                        _device_name);
                    continue;
                }

                if (format_range_->MaximumChannels > std::numeric_limits<uint16_t>::max()) {
                    hi_log_error(
                        "Bad KSDATARANGE_AUDIO MaximumChannels == {} for device {}",
                        format_range_->MaximumChannels,
                        _device_name);
                    continue;
                }

                if (format_range_->MinimumSampleFrequency > format_range_->MaximumSampleFrequency) {
                    hi_log_error(
                        "Bad KSDATARANGE_AUDIO MinimumSampleFrequency == {}, MaximumSampleFrequency {} for device {}",
                        format_range_->MinimumSampleFrequency,
                        format_range_->MaximumSampleFrequency,
                        _device_name);
                    continue;
                }

                hilet num_bits_first = format_range_->MinimumBitsPerSample;
                hilet num_bits_last = format_range_->MaximumBitsPerSample;
                hilet num_channels = narrow_cast<uint16_t>(format_range_->MaximumChannels);
                hilet min_sample_rate = narrow_cast<uint32_t>(format_range_->MinimumSampleFrequency);
                hilet max_sample_rate = narrow_cast<uint32_t>(format_range_->MaximumSampleFrequency);

                // There are only very few sample-formats that a device will actually support, therefor
                // the audio-format-range discretized them. Very likely the audio device driver will be lying.
                for (auto num_bits = num_bits_first; num_bits <= num_bits_last; ++num_bits) {
                    hilet num_bytes = narrow_cast<uint8_t>((num_bits + 7) / 8);
                    if (has_int) {
                        hilet num_minor_bits = narrow_cast<uint8_t>(num_bits - 1);
                        hilet sample_format = pcm_format{false, std::endian::native, true, num_bytes, 0, num_minor_bits};
                        co_yield audio_format_range{
                            sample_format, num_channels, min_sample_rate, max_sample_rate, surround_mode::none};
                    }
                    if (has_float and num_bits == 32) {
                        hilet sample_format = pcm_format{true, std::endian::native, true, num_bytes, 8, 23};
                        co_yield audio_format_range{
                            sample_format, num_channels, min_sample_rate, max_sample_rate, surround_mode::none};
                    }
                }
            }
        }
    }

    /** Get all the audio formats supported by this device.
     *
     * @note The audio device is very likely lying about its capabilities, the resulting
     *       formats should be filtered through `IAudioClient::IsFormatSupported()`.
     * @param direction The audio direction for which the formats will be enumerated.
     * @return A set of audio format ranges that this audio device supports.
     */
    [[nodiscard]] generator<audio_format_range> get_format_ranges(audio_direction direction) const noexcept
    {
        for (auto pin_nr : find_streaming_pins(direction)) {
            for (hilet& range : get_format_ranges(pin_nr)) {
                co_yield range;
            }
        }
    }

    template<typename T>
    [[nodiscard]] generator<T const *> get_pin_properties(ULONG pin_id, KSPROPERTY_PIN property) const
    {
        auto r = get_pin_property_data(pin_id, property);
        auto *ptr = r.get();

        auto *header = std::launder(reinterpret_cast<KSMULTIPLE_ITEM const *>(ptr));

        hilet expected_size = header->Count * sizeof(T) + sizeof(KSMULTIPLE_ITEM);
        if (header->Size != expected_size) {
            throw io_error("KSMULTIPLE_ITEM header corrupt");
        }

        ptr += sizeof(KSMULTIPLE_ITEM);
        for (auto i = 0_uz; i != header->Count; ++i) {
            co_yield std::launder(reinterpret_cast<T const *>(ptr));
            ptr += sizeof(T);
        }
    }

    /**
     *
     * @note KSDATARANGE is same as KSDATAFORMAT.
     */
    template<>
    [[nodiscard]] generator<KSDATARANGE const *> get_pin_properties(ULONG pin_id, KSPROPERTY_PIN property) const
    {
        auto r = get_pin_property_data(pin_id, property);
        auto *ptr = r.get();

        auto *header = std::launder(reinterpret_cast<KSMULTIPLE_ITEM const *>(ptr));

        ptr += sizeof(KSMULTIPLE_ITEM);
        for (auto i = 0_uz; i != header->Count; ++i) {
            auto *item = std::launder(reinterpret_cast<KSDATARANGE const *>(ptr));
            co_yield item;

            ptr += item->FormatSize;
        }
    }

private:
    std::string _device_name;
    HANDLE _handle;

    [[nodiscard]] std::string get_pin_property_string(ULONG pin_id, KSPROPERTY_PIN property) const;

    template<typename T>
    [[nodiscard]] T get_pin_property(ULONG pin_id, KSPROPERTY_PIN property) const
    {
        auto property_info = KSP_PIN{};
        property_info.Property.Set = KSPROPSETID_Pin;
        property_info.Property.Id = property;
        property_info.Property.Flags = KSPROPERTY_TYPE_GET;
        property_info.PinId = pin_id;
        property_info.Reserved = 0;

        DWORD r_size;
        T r;
        if (not DeviceIoControl(_handle, IOCTL_KS_PROPERTY, &property_info, sizeof(KSP_PIN), &r, sizeof(T), &r_size, NULL)) {
            throw io_error(get_last_error_message());
        }

        if (r_size != sizeof(T)) {
            throw io_error("Unexpected return size");
        }

        return r;
    }

    std::unique_ptr<char[]> get_pin_property_data(ULONG pin_id, KSPROPERTY_PIN property) const
    {
        auto property_info = KSP_PIN{};
        property_info.Property.Set = KSPROPSETID_Pin;
        property_info.Property.Id = property;
        property_info.Property.Flags = KSPROPERTY_TYPE_GET;
        property_info.PinId = pin_id;
        property_info.Reserved = 0;

        DWORD r_size;
        if (DeviceIoControl(_handle, IOCTL_KS_PROPERTY, &property_info, sizeof(KSP_PIN), NULL, 0, &r_size, NULL)) {
            throw io_error("Unexpected return size 0");
        }
        if (GetLastError() != ERROR_MORE_DATA) {
            throw io_error(get_last_error_message());
        }

        if (r_size < sizeof(KSMULTIPLE_ITEM)) {
            throw io_error("Unexpected return size");
        }

        auto r = std::make_unique<char[]>(r_size);
        if (not DeviceIoControl(_handle, IOCTL_KS_PROPERTY, &property_info, sizeof(KSP_PIN), r.get(), r_size, &r_size, NULL)) {
            throw io_error(get_last_error_message());
        }

        if (r_size < sizeof(KSMULTIPLE_ITEM)) {
            throw io_error("Incomplete header read");
        }

        auto *header = std::launder(reinterpret_cast<KSMULTIPLE_ITEM *>(r.get()));
        if (r_size < header->Size) {
            throw io_error("Incomplete read");
        }

        return r;
    }

    template<>
    [[nodiscard]] std::string get_pin_property<std::string>(ULONG pin_id, KSPROPERTY_PIN property) const
    {
        auto property_info = KSP_PIN{};
        property_info.Property.Set = KSPROPSETID_Pin;
        property_info.Property.Id = property;
        property_info.Property.Flags = KSPROPERTY_TYPE_GET;
        property_info.PinId = pin_id;
        property_info.Reserved = 0;

        DWORD r_size;
        if (DeviceIoControl(_handle, IOCTL_KS_PROPERTY, &property_info, sizeof(KSP_PIN), NULL, 0, &r_size, NULL)) {
            return std::string{};
        }

        if (GetLastError() != ERROR_MORE_DATA) {
            throw io_error(get_last_error_message());
        }

        if (r_size % 2 != 0) {
            throw io_error("Expected even number of bytes in return value.");
        }

        auto r = std::wstring(r_size / sizeof(wchar_t{}) - 1, wchar_t{});
        hi_assert(r_size == (r.size() + 1) * sizeof(wchar_t{}));

        if (not DeviceIoControl(_handle, IOCTL_KS_PROPERTY, &property_info, sizeof(KSP_PIN), r.data(), r_size, &r_size, NULL)) {
            throw io_error(get_last_error_message());
        }

        return hi::to_string(r);
    }

    [[nodiscard]] bool is_streaming_interface(ULONG pin_nr) const noexcept
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

    [[nodiscard]] bool is_standerdio_medium(ULONG pin_nr) const noexcept
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

    [[nodiscard]] bool is_streaming_pin(ULONG pin_nr, audio_direction direction) const noexcept
    {
        // Check if this is a streaming-pin.
        if (not is_streaming_interface(pin_nr)) {
            return false;
        }

        if (not is_standerdio_medium(pin_nr)) {
            return false;
        }

        // Check if the dataflow direction of the pin is in the opposite-direction of the end-point.
        switch (get_pin_property<KSPIN_DATAFLOW>(pin_nr, KSPROPERTY_PIN_DATAFLOW)) {
        case KSPIN_DATAFLOW_OUT:
            if (direction != audio_direction::input and direction != audio_direction::bidirectional) {
                return false;
            }
            break;
        case KSPIN_DATAFLOW_IN:
            if (direction != audio_direction::output and direction != audio_direction::bidirectional) {
                return false;
            }
            break;
        default:
            hi_no_default();
        }

        // Modern device drivers seem no longer to support directly streaming samples through this API, therefor those pins
        // can no longer communicate at all, but can still be interrogated for the audio formats it supports.
        // The Scarlett 2i2 has streaming pins that can be interrogated but are KSPIN_COMMUNICATION_NONE.
        // Therefor the old examples on the web that check for KSPROPERTY_PIN_COMMUNICATION are no longer valid.

        return true;
    }
};

}} // namespace hi::inline v1
