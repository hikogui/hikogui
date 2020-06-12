// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/FileMapping.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/memory.hpp"
#include "TTauri/Foundation/required.hpp"
#include <mutex>

namespace tt {


std::shared_ptr<File> FileMapping::findOrOpenFile(URL const& location, AccessMode accessMode)
{
    static std::mutex mutex;
    static std::unordered_map<URL, std::vector<std::weak_ptr<File>>> mappedFiles;

    let lock = std::scoped_lock(mutex);

    cleanupWeakPointers(mappedFiles);

    // We want files to be freshly created if it did not exist before.
    auto& files = mappedFiles[location];
    for (let &weak_file : files) {
        if (auto file = weak_file.lock()) {
            if (file->accessMode >= accessMode) {
                return file;
            }
        }
    }

    auto file = std::make_shared<File>(location, accessMode);
    files.push_back(file);
    return file;
}

}
