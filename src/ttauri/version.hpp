// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>

namespace tt {

struct version {
    /** Incremented on backward incompatible change. */
    int major = 0;
    /** Incremented on additive change */
    int minor = 2;
    /** Incremented on bug fixes */
    int patch = 3;

    /** Name of the application or library. */
    std::string name = "ttauri";

    /** The version string with major.minor.patch, commits since tag, short hash of tag. */
    std::string version_long = "0.2.3-18-g7443ac";

    /** A version string containing only major.minor.patch version numbers. */
    std::string version_short = "0.2.3";

    /* The git tag RCS. */
    std::string git_tag_rcs = "v0.2.3-18-g7443aca0bec31b4c9c7f3b51da71f16365ec8e9e-dirty";

    /** Name of the git branch. */
    std::string git_branch = "relative-clipping-rectangle";

    /** The git commit short hash. */
    std::string git_commit = "7443aca";

    /** The number of commits since the version tag. */
    int git_commits_since_version_tag = 18;

    /** There are local changes. */
    bool git_local_changes = "dirty";

    /** The release date shows when the release was tagged. */
    std::string release_date = "2021-02-27";

};

inline version ttauri_version;
inline version application_version;

}
