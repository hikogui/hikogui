// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"
#include "base.hpp"
#include <expected>
#include <string>
#include <system_error>
#include <filesystem>

hi_export_module(hikogui.win32.libloaderapi);

hi_export namespace hi {
inline namespace v1 {

[[nodiscard]] hi_inline std::expected<std::filesystem::path, win32_error> win32_GetModuleFileName(HMODULE module_handle = NULL) noexcept
{
    std::wstring module_path;
    auto buffer_size = MAX_PATH; // initial default value = 256
    static_assert(MAX_PATH != 0);

    // iterative buffer resizing to max value of 32768 (256*2^7)
    for (std::size_t i = 0; i < 7; ++i) {
        module_path.resize(buffer_size);
        auto num_chars = GetModuleFileNameW(module_handle, module_path.data(), buffer_size);
        if (num_chars == 0) {
            return std::unexpected{win32_GetLastError()};
        
        } else if (num_chars < module_path.length()) {
            module_path.resize(num_chars);
            return std::filesystem::path{module_path};

        } else {
            buffer_size *= 2;
        }
    }
    return std::unexpected{win32_error::insufficient_buffer};
}

} // namespace v1
}
