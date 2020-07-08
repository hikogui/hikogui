// Copyright 2020 Pokitec
// All rights reserved.

#include "globals.hpp"
#include "URL.hpp"
#include "strings.hpp"
#include "required.hpp"
#include "url_parser.hpp"
#include <regex>

#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/dir.h>

namespace tt {

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
        case DT_REG:
            filenames.push_back(std::move(filename));
            break;
        default:;
        }
    }
    closedir(dirp);

    return filenames;
}

}
