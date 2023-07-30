// Copyright Take Vos 2020-2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "URL.hpp"
#include "semantic_version.hpp"
#include "../utility/module.hpp"
#include "../macros.hpp"
#include <atomic>

namespace hi { inline namespace v1 {

[[nodiscard]] constexpr std::string get_library_name() noexcept
{
    return "HikoGUI Library";
}

[[nodiscard]] constexpr std::string get_library_slug() noexcept
{
    return "hikogui";
}

[[nodiscard]] constexpr std::string get_library_vendor_name() noexcept
{
    return "HikoGUI";
}

[[nodiscard]] constexpr semantic_version get_library_version() noexcept
{
    return semantic_version{0, 7, 0};
}

[[nodiscard]] constexpr std::string get_library_license() noexcept
{
    return "BSL-1.0";
}

[[nodiscard]] constexpr URL get_library_url() noexcept
{
    return URL{"https://github.com/hikogui/hikogui"};
}

[[nodiscard]] constexpr std::string get_library_description() noexcept
{
    return "A portable, low latency, retained-mode GUI framework written in C++.";
}

}} // namespace hi::v1
