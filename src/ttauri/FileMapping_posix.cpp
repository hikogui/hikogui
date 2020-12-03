// Copyright 2019 Pokitec
// All rights reserved.

#include "FileMapping.hpp"
#include "exception.hpp"
#include "logger.hpp"
#include "memory.hpp"
#include "required.hpp"
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
