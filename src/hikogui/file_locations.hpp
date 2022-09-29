

/** @file file_locations.hpp function to locate files and directories.
 */

#pragma once

#include <filesystem>

namespace hi
inline namespace v1 {

[[nodiscard]] std::filesystem::path resource_directory() noexcept;
[[nodiscard]] std::filesystem::path executable_directory() noexcept;
[[nodiscard]] std::filesystem::path executable_path() noexcept;
[[nodiscard]] std::filesystem::path application_data_directory() noexcept;
[[nodiscard]] std::filesystem::path application_log_directory() noexcept;
[[nodiscard]] std::filesystem::path system_font_directory() noexcept;
[[nodiscard]] std::filesystem::path application_preferences_path() noexcept;
[[nodiscard]] std::vector<std::filesystem::path> font_directories() noexcept;

}}

