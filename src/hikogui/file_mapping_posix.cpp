// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "file_mapping.hpp"
#include "exception.hpp"
#include "log.hpp"
#include "memory.hpp"
#include "utility.hpp"
#include <mutex>

namespace hi::inline v1 {

file_mapping::file_mapping(std::shared_ptr<file> const &file, std::size_t size) :
    file(file), size(size > 0 ? size : file::fileSize(file->location))
{
}

file_mapping::file_mapping(URL const &location, access_mode access_mode, std::size_t size) :
    file_mapping(findOrOpenFile(location, access_mode), size)
{
}

file_mapping::~file_mapping() {}

} // namespace hi::inline v1
