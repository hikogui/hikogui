// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "win32_headers.hpp"

#include "file_mapping.hpp"
#include "exception.hpp"
#include "log.hpp"
#include "memory.hpp"
#include "utility.hpp"
#include <mutex>

namespace hi::inline v1 {

file_mapping::file_mapping(std::shared_ptr<hi::file> const &file, std::size_t size) :
    file(file), size(size > 0 ? size : file->size())
{
    DWORD protect;
    if (any(accessMode() & access_mode::read) and any(accessMode() & access_mode::write)) {
        protect = PAGE_READWRITE;
    } else if (any(accessMode() & access_mode::read)) {
        protect = PAGE_READONLY;
    } else {
        throw io_error(std::format("{}: Illegal access mode WRONLY/0 when mapping file.", location()));
    }

    DWORD maximumSizeHigh = this->size >> 32;
    DWORD maximumSizeLow = this->size & 0xffffffff;

    if (this->size == 0) {
        mapHandle = nullptr;
    } else {
        if ((mapHandle = CreateFileMappingW(file->_file_handle, NULL, protect, maximumSizeHigh, maximumSizeLow, nullptr)) ==
            nullptr) {
            throw io_error(std::format("{}: Could not create file mapping. '{}'", location(), get_last_error_message()));
        }
    }
}

file_mapping::file_mapping(URL const &location, access_mode accessMode, std::size_t size) :
    file_mapping(findOrOpenFile(location, accessMode), size)
{
}

file_mapping::~file_mapping()
{
    if (mapHandle != nullptr) {
        if (!CloseHandle(mapHandle)) {
            hi_log_fatal("Could not close file mapping object on file '{}'", get_last_error_message());
        }
    }
}

} // namespace hi::inline v1
