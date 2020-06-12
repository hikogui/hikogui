// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/FileMapping.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/memory.hpp"
#include "TTauri/Foundation/required.hpp"
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
