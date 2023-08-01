// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include <string>
#include <string_view>

hi_export_module(hikogui.metadata.semantic_version);

hi_export namespace hi::inline v1 {

class semantic_version {
public:
    /** Incremented on backward incompatible change. */
    int major;
    /** Incremented on additive change */
    int minor;
    /** Incremented on bug fixes */
    int patch;

    constexpr semantic_version() noexcept : major(0), minor(0), patch(0) {}

    constexpr semantic_version(int major, int minor, int patch) noexcept : major(major), minor(minor), patch(patch) {}
};

} // namespace hi::inline v1
