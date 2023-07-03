

#pragma once

#include "utility/module.hpp"
#include <optional>
#include <string>
#include <string_view>
#include <cstdlib>

namespace hi { inline namespace v1 {

[[nodiscard]] std::optional<std::string> get_user_setting_string(std::string_view key);
[[nodiscard]] std::optional<long long> get_user_setting_integral(std::string_view key);

/** Get a user-setting for the application.
 *
 * Keys starting with two consequitive underscores are reserved by the HikoGUI library.
 *
 * @param key A key for the user setting.
 * @return The value from the user-settings.
 * @retval std::nullopt The key was not found in the user-settings.
 * @throws std::invalid_argument When the key is not valid.
 * @throws std::out_of_range if the value in the default does not fit in the return value.
 */
template<typename T>
[[nodiscard]] std::optional<T> get_user_setting(std::string_view key) = delete;

/** Set a user-setting for the application.
 *
 * Keys starting with two consequitive underscores are reserved by the HikoGUI library.
 *
 * @param key A key for the user setting.
 * @param value The value to set.
 * @throws std::invalid_argument When the key is not valid.
 * @throws std::out_of_range if the value in the default does not fit in the return value.
 */
void set_user_setting(std::string_view key, std::string_view value);

/** Set a user-setting for the application.
 *
 * Keys starting with two consequitive underscores are reserved by the HikoGUI library.
 *
 * @param key A key for the user setting.
 * @param value The value to set.
 * @throws std::invalid_argument When the key is not valid.
 * @throws std::out_of_range if the value in the default does not fit in the return value.
 */
void set_user_setting(std::string_view key, long long value);

/** Delete a user-setting for the application.
 *
 * Keys starting with two consequitive underscores are reserved by the HikoGUI library.
 *
 * @param key A key for the user setting.
 * @throws std::invalid_argument When the key is not valid.
 * @throws std::out_of_range if the value in the default does not fit in the return value.
 */
void delete_user_setting(std::string_view key);

/** Delete all user-setting for the application.
 */
void delete_user_settings();

template<>
[[nodiscard]] inline std::optional<std::string> get_user_setting(std::string_view key)
{
    return get_user_setting_string(key);
}

template<std::integral T>
[[nodiscard]] inline std::optional<T> get_user_setting(std::string_view key)
{
    if (hilet value = get_user_setting_integral(key)) {
        if (can_narrow_cast<T>(*value)) {
            return narrow_cast<T>(*value);
        } else {
            throw std::out_of_range(std::format("Integer {} out of range", *value));
        }
    } else {
        return std::nullopt;
    }
}

}} // namespace hi::v1
