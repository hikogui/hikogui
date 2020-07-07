// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/foundation/FileMapping.hpp"
#include "ttauri/foundation/exceptions.hpp"
#include "ttauri/foundation/logger.hpp"
#include "ttauri/foundation/memory.hpp"
#include "ttauri/foundation/required.hpp"
#include <mutex>

namespace tt {

FileMapping::FileMapping(std::shared_ptr<File> const& file, size_t size) :
    file(file), size(size > 0 ? size : File::fileSize(file->location))
{
}

FileMapping::FileMapping(URL const &location, AccessMode accessMode, size_t size) :
    FileMapping(findOrOpenFile(location, accessMode), size) {}

FileMapping::~FileMapping()
{
}

}
