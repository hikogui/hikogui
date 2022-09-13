// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "file_mapping.hpp"
#include "exception.hpp"
#include "log.hpp"
#include "memory.hpp"
#include "utility.hpp"
#include "unfair_mutex.hpp"
#include <mutex>

namespace hi::inline v1 {

std::shared_ptr<file> file_mapping::findOrOpenFile(URL const &location, access_mode accessMode)
{
    static unfair_mutex mutex;
    static std::unordered_map<URL, std::vector<std::weak_ptr<hi::file>>> mappedFiles;

    hilet lock = std::scoped_lock(mutex);

    cleanupWeakPointers(mappedFiles);

    // We want files to be freshly created if it did not exist before.
    auto &files = mappedFiles[location];
    for (hilet &weak_file : files) {
        if (auto file = weak_file.lock()) {
            if ((file->_access_mode & accessMode) == accessMode) {
                return file;
            }
        }
    }

    auto file = std::make_shared<hi::file>(location, accessMode);
    files.push_back(file);
    return file;
}

} // namespace hi::inline v1
