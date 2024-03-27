// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <filesystem>
#include <string>
#include <fstream>

hi_export_module(hikogui.path.cmake_install);

hi_export namespace hi { inline namespace v1 {

struct cmake_install {
    std::filesystem::path source_dir;
};

/** Parse a cmake_install.cmake file.
 * 
 * @param path The path to the cmake_install.cmake file.
 * @return The path to the source dir
 * @retval std::nullopt if the file does not exist,
 *         or could not be parsed, or the source-dir does not exist. 
*/
[[nodiscard]] hi_inline std::optional<cmake_install> parse_cmake_install(std::filesystem::path path) noexcept
{
    if (not std::filesystem::exists(path)) {
        return std::nullopt;
    }

    auto line = std::string{};
    try {
        auto fd = std::ifstream{path.string()};
        line = getline(fd, 512);
        fd.close();

    } catch (...) {
        return std::nullopt;
    }

    auto const cmake_install_start = std::string{"# Install script for directory: "};
    if (not line.starts_with(cmake_install_start)) {
        return std::nullopt;
    }

    auto source_dir = std::filesystem::path{line.substr(cmake_install_start.size())};
    if (not std::filesystem::exists(source_dir)) {
        return std::nullopt;
    }

    return cmake_install{source_dir};
}

}}
