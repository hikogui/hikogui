// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>

namespace tt {

struct version {
    /** Name of the application or library. */
    std::string name;

    /** Incremented on backward incompatible change. */
    int major;
    /** Incremented on additive change */
    int minor;
    /** Incremented on bug fixes */
    int patch;

    version(std::string_view name, int major, int minor, int patch) noexcept :
        name(name), major(major), minor(minor), patch(patch) {}
};

}
