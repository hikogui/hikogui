// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "file_mapping.hpp"
#include "exception.hpp"
#include "logger.hpp"
#include "memory.hpp"
#include "required.hpp"
#include <mutex>

namespace tt {


std::shared_ptr<file> file_mapping::findOrOpenFile(URL const& location, access_mode accessMode)
{
    static std::mutex mutex;
    static std::unordered_map<URL, std::vector<std::weak_ptr<tt::file>>> mappedFiles;

    ttlet lock = std::scoped_lock(mutex);

    cleanupWeakPointers(mappedFiles);

    // We want files to be freshly created if it did not exist before.
    auto& files = mappedFiles[location];
    for (ttlet &weak_file : files) {
        if (auto file = weak_file.lock()) {
            if (file->_access_mode >= accessMode) {
                return file;
            }
        }
    }

    auto file = std::make_shared<tt::file>(location, accessMode);
    files.push_back(file);
    return file;
}

}
