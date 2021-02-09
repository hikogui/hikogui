// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>

namespace tt {

struct version {
    /** Incremented on backward incompatible change. */
    int major;
    /** Incremented on additive change */
    int minor;
    /** Incremented on bug fixes */
    int patch;

    /** Name of the application or library */
    std::string name;

    /** Name of the git branch. */
    std::string git_branch;

    /** The git sha */
    std::string git_commit;

    /** The number of commits since the version tag. */
    int git_commits_since_version_tag;

    /** There are local changes. */
    bool git_local_changes;
};

inline version ttauri_version;
inline version application_version;

}
