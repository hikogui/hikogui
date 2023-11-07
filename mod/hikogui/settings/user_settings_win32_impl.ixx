
module;
#include "../macros.hpp"

#include <format>
#include <string>
#include <expected>
#include <system_error>
#include <Windows.h>
#include <winreg.h>

export module hikogui_settings_user_settings : impl;
import : intf;
import hikogui_metadata;
import hikogui_path;
import hikogui_win32;

export namespace hi { inline namespace v1 {

[[nodiscard]] std::string user_setting_registry_path()
{
    return std::format("Software\\{}\\{}", get_application_vendor(), get_application_name());
}

/** 
 * 
 * @return A string value, or std::errc::file_not_found if entry not found, or other error.
 */
[[nodiscard]] std::expected<std::string, std::error_code> get_user_setting_string(std::string_view name) noexcept
{
    // First check the registry of the current-user.
    if (hilet value = win32_RegGetValue<std::string>(HKEY_CURRENT_USER, user_setting_registry_path(), name)) {
        return *value;
    } else if (value.error() != win32_error::file_not_found) {
        return std::unexpected{std::error_code{value.error()}};
    }

    // Now check the registry for the local-machine.
    // These are settings that where made by the Administrator of the machine.
    if (hilet value = win32_RegGetValue<std::string>(HKEY_LOCAL_MACHINE, user_setting_registry_path(), name)) {
        return *value;
    } else {
        return std::unexpected{std::error_code{value.error()}};
    }
}

[[nodiscard]] std::expected<long long, std::error_code> get_user_setting_integral(std::string_view name) noexcept
{
    // First check the registry of the current-user.
    if (hilet value = win32_RegGetValue<long long>(HKEY_CURRENT_USER, user_setting_registry_path(), name)) {
        return *value;
    } else if (value.error() != win32_error::file_not_found) {
        return std::unexpected{std::error_code{value.error()}};
    }

    // Now check the registry for the local-machine.
    // These are settings that where made by the Administrator of the machine.
    if (hilet value = win32_RegGetValue<long long>(HKEY_LOCAL_MACHINE, user_setting_registry_path(), name)) {
        return *value;
    } else {
        return std::unexpected{std::error_code{value.error()}};
    }
}

std::error_code set_user_setting(std::string_view name, std::string_view value) noexcept
{
    return win32_RegSetKeyValue(HKEY_CURRENT_USER, user_setting_registry_path(), name, value);
}

std::error_code set_user_setting(std::string_view name, long long value) noexcept
{
    return win32_RegSetKeyValue(HKEY_CURRENT_USER, user_setting_registry_path(), name, narrow_cast<uint32_t>(value));
}

std::error_code delete_user_setting(std::string_view name) noexcept
{
    return win32_RegDeleteKeyValue(HKEY_CURRENT_USER, user_setting_registry_path(), name);
}

std::error_code delete_user_settings() noexcept
{
    return win32_RegDeleteKey(HKEY_CURRENT_USER, user_setting_registry_path());
}

}}

