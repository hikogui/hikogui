// Copyright 2019 Pokitec
// All rights reserved.

#include "FileView.hpp"
#include "exceptions.hpp"
#include "logger.hpp"
#include "memory.hpp"
#include "URL.hpp"
#include "required.hpp"
#include <mutex>

namespace tt {

std::shared_ptr<FileMapping> FileView::findOrCreateFileMappingObject(URL const& location, access_mode accessMode, size_t size)
{
    static std::mutex mutex;
    static std::unordered_map<URL, std::vector<std::weak_ptr<FileMapping>>> mappedFileObjects;

    ttlet lock = std::scoped_lock(mutex);

    cleanupWeakPointers(mappedFileObjects);

    auto& mappings = mappedFileObjects[location];

    for (auto weak_fileMappingObject : mappings) {
        if (auto fileMappingObject = weak_fileMappingObject.lock()) {
            if (fileMappingObject->size >= size && fileMappingObject->accessMode() >= accessMode) {
                return fileMappingObject;
            }
        }
    }

    auto fileMappingObject = std::make_shared<FileMapping>(location, accessMode, size);
    mappings.push_back(fileMappingObject);
    return fileMappingObject;
}

}
