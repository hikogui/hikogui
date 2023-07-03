// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility/module.hpp"
#include <string_view>
#include <string>
#include <vector>
#include <cstdint>
#include <optional>

namespace hi::inline v1 {

enum class registry_key { classes_root, current_config, current_user, local_machine, users };

/** Read from the registry value.
 *
 * @param key The registry's key
 * @param path The path to the values.
 * @param name The name of the value.
 * @throws hi::os_error Unable to delete the registry value.
 */
[[nodiscard]] void registry_delete(registry_key key, std::string_view path, std::string_view name);

/** Write a DWORD registry value.
 *
 * @note If the path or name do not exist it is automatically created.
 * @param key The registry's key
 * @param path The path to the values.
 * @param name The name of the value.
 * @param value The value to write
 * @throws hi::os_error Unable to write the registry-value.
 */
[[nodiscard]] void registry_write(registry_key key, std::string_view path, std::string_view name, uint32_t value);


/** Write a string registry value.
 *
 * @note If the path or name do not exist it is automatically created.
 * @param key The registry's key
 * @param path The path to the values.
 * @param name The name of the value.
 * @param value The value to write
 * @throws hi::os_error Unable to write the registry-value.
 */
[[nodiscard]] void registry_write(registry_key key, std::string_view path, std::string_view name, std::string_view value);

/** Read a DWORD registry value.
 *
 * @param key The registry's key
 * @param path The path to the values.
 * @param name The name of the value.
 * @return value, or std::nullopt if the registry-value was not found.
 * @throws hi::os_error Unable to read the registry-value, for example when the type was different.
 */
[[nodiscard]] std::optional<uint32_t> registry_read_dword(registry_key key, std::string_view path, std::string_view name);

/** Read a strings from the registry value.
 *
 * @param key The registry's key
 * @param path The path to the values.
 * @param name The name of the value.
 * @return value, or std::nullopt if the registry-value was not found.
 * @throws hi::os_error Unable to read the registry-value, for example when the type was different.
 */
[[nodiscard]] std::optional<std::string> registry_read_string(registry_key key, std::string_view path, std::string_view name);

/** Read a list of strings from the registry value.
 *
 * @param key The registry's key
 * @param path The path to the values.
 * @param name The name of the value.
 * @return value, or std::nullopt if the registry-value was not found.
 * @throws hi::os_error Unable to read the registry-value, for example when the type was different.
 */
[[nodiscard]] std::optional<std::vector<std::string>>
registry_read_multi_string(registry_key key, std::string_view path, std::string_view name);

/** Read from the registry value.
 *
 * @tparam T The type of the value to read.
 * @param key The registry's key
 * @param path The path to the values.
 * @param name The name of the value.
 * @return value, or std::nullopt if the registry-value was not found.
 * @throws hi::os_error Unable to read the registry-value, for example when the type was different.
 */
template<typename T>
[[nodiscard]] std::optional<T> registry_read(registry_key key, std::string_view path, std::string_view name) = delete;

template<std::integral T>
[[nodiscard]] inline std::optional<T> registry_read(registry_key key, std::string_view path, std::string_view name)
{
    if (hilet tmp = registry_read_dword(key, path, name)) {
        return narrow_cast<T>(*tmp);
    } else {
        return std::nullopt;
    }
}

template<>
[[nodiscard]] inline std::optional<std::string> registry_read(registry_key key, std::string_view path, std::string_view name)
{
    return registry_read_string(key, path, name);
}

template<>
[[nodiscard]] inline std::optional<std::vector<std::string>>
registry_read(registry_key key, std::string_view path, std::string_view name)
{
    return registry_read_multi_string(key, path, name);
}

} // namespace hi::inline v1
