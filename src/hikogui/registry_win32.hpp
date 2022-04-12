// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string_view>
#include <string>
#include <vector>
#include <cstdint>

namespace tt::inline v1 {

/** Read a DWORD registry value from the HKEY_CURRENT_USER.
 *
 * @throws tt::os_error when the path/name is not found in the registry.
 */
[[nodiscard]] uint32_t registry_read_current_user_dword(std::string_view path, std::string_view name);

/** Read a list of strings from the registry value from the HKEY_CURRENT_USER.
 *
 * @throws tt::os_error when the path/name is not found in the registry.
 */
[[nodiscard]] std::vector<std::string> registry_read_current_user_multi_string(std::string_view path, std::string_view name);

} // namespace tt::inline v1
