// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"
#include "hresult_error_intf.hpp"
#include <expected>
#include <string>
#include <system_error>
#include <filesystem>

hi_export_module(hikogui.win32 : shlobj_core);

hi_export namespace hi {
inline namespace v1 {

/** Convenience function for SHGetKnownFolderPath().
 *  Retrieves a full path of a known folder identified by the folder's KNOWNFOLDERID.
 *  See https://docs.microsoft.com/en-us/windows/win32/shell/knownfolderid#constants
 *
 * @param KNOWNFOLDERID folder_id.
 * @return The path of the folder.
 */
[[nodiscard]] inline std::expected<std::filesystem::path, hresult_error> win32_SHGetKnownFolderPath(KNOWNFOLDERID const& folder_id) noexcept
{
    PWSTR wpath = nullptr;
    auto const result_code = SHGetKnownFolderPath(folder_id, 0, nullptr, &wpath);

    if (result_code != S_OK) {
        return std::unexpected{static_cast<hresult_error>(result_code)};
    }

    auto r = std::filesystem::path{wpath} / "";
    CoTaskMemFree(wpath);
    return r;
}

} // namespace v1
}
