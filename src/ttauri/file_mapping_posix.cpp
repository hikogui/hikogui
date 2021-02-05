// Copyright 2019 Pokitec
// All rights reserved.

#include "file_mapping.hpp"
#include "exception.hpp"
#include "logger.hpp"
#include "memory.hpp"
#include "required.hpp"
#include <mutex>

namespace tt {

file_mapping::file_mapping(std::shared_ptr<File> const& file, size_t size) :
    file(file), size(size > 0 ? size : File::fileSize(file->location))
{
}

file_mapping::file_mapping(URL const &location, AccessMode accessMode, size_t size) :
    file_mapping(findOrOpenFile(location, accessMode), size) {}

file_mapping::~file_mapping()
{
}

}
