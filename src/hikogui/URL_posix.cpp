// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "URL.hpp"
#include "strings.hpp"
#include "utility.hpp"
#include "url_parser.hpp"
#include <regex>

#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/dir.h>

namespace hi::inline v1 {

std::vector<std::string> URL::filenamesByScanningDirectory(std::string_view path) noexcept
{
    std::vector<std::string> filenames;

    auto dirp = opendir(path.data());
    if (dirp == nullptr) {
        return filenames;
    }

    struct dirent *dp;
    while ((dp = readdir(dirp)) != nullptr) {
        auto filename = std::string(dp->d_name, dp->d_namlen);

        switch (dp->d_type) {
        case DT_DIR:
            if (filename == "." || filename == "..") {
                continue;
            }
            filename += '/';
            [[fallthrough]];
        case DT_REG: filenames.push_back(std::move(filename)); break;
        default:;
        }
    }
    closedir(dirp);

    return filenames;
}

} // namespace hi::inline v1
