// Copyright 2019 Pokitec
// All rights reserved.

#include "file_view.hpp"
#include "exception.hpp"
#include "logger.hpp"
#include "memory.hpp"
#include "URL.hpp"
#include "required.hpp"
#include <mutex>

namespace tt {

std::shared_ptr<file_mapping> file_view::findOrCreateFileMappingObject(URL const& location, access_mode accessMode, size_t size)
{
    static std::mutex mutex;
    static std::unordered_map<URL, std::vector<std::weak_ptr<file_mapping>>> mappedFileObjects;

    ttlet lock = std::scoped_lock(mutex);

    cleanupWeakPointers(mappedFileObjects);

    auto& mappings = mappedFileObjects[location];

    for (auto weak__file_mapping_object : mappings) {
        if (auto _file_mapping_object = weak__file_mapping_object.lock()) {
            if (_file_mapping_object->size >= size && _file_mapping_object->accessMode() >= accessMode) {
                return _file_mapping_object;
            }
        }
    }

    auto _file_mapping_object = std::make_shared<file_mapping>(location, accessMode, size);
    mappings.push_back(_file_mapping_object);
    return _file_mapping_object;
}

}
