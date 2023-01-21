// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_direction.hpp"
#include "audio_format_range.hpp"
#include "../generator.hpp"
#include "../utility/module.hpp"
#include "../log.hpp"
#include <string>

namespace hi::inline v1 {

class win32_device_interface {
public:
    win32_device_interface(std::string device_name);
    ~win32_device_interface();

    [[nodiscard]] ULONG pin_count() const noexcept;
    [[nodiscard]] std::string pin_name(ULONG pin_nr) const noexcept;

    [[nodiscard]] generator<ULONG> find_streaming_pins(audio_direction direction) const noexcept;

    [[nodiscard]] GUID pin_category(ULONG pin_nr) const noexcept;

    [[nodiscard]] KSPIN_COMMUNICATION pin_communication(ULONG pin_nr) const noexcept;

    [[nodiscard]] generator<audio_format_range> get_format_ranges(ULONG pin_nr) const noexcept;

    /** Get all the audio formats supported by this device.
     *
     * @note The audio device is very likely lying about its capabilities, the resulting
     *       formats should be filtered through `IAudioClient::IsFormatSupported()`.
     * @param direction The audio direction for which the formats will be enumerated.
     * @return A set of audio format ranges that this audio device supports.
     */
    [[nodiscard]] generator<audio_format_range> get_format_ranges(audio_direction direction) const noexcept;

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

    [[nodiscard]] bool is_streaming_interface(ULONG pin_nr) const noexcept;

    [[nodiscard]] bool is_standerdio_medium(ULONG pin_nr) const noexcept;

    [[nodiscard]] bool is_streaming_pin(ULONG pin_nr, audio_direction direction) const noexcept;
};

} // namespace hi::inline v1
