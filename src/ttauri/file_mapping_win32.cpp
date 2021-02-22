// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "file_mapping.hpp"
#include "exception.hpp"
#include "logger.hpp"
#include "memory.hpp"
#include "required.hpp"
#include <mutex>
#include <Windows.h>

namespace tt {

file_mapping::file_mapping(std::shared_ptr<tt::file> const &file, size_t size) :
    file(file), size(size > 0 ? size : file::file_size(file->_location))
{
    DWORD protect;
    if (accessMode() >= (access_mode::read | access_mode::write)) {
        protect = PAGE_READWRITE;
    }
    else if (accessMode() >= access_mode::read) {
        protect = PAGE_READONLY;
    }
    else {
        throw io_error("{}: Illegal access mode WRONLY/0 when mapping file.", location());
    }

    DWORD maximumSizeHigh = this->size >> 32;
    DWORD maximumSizeLow = this->size & 0xffffffff;

    if (this->size == 0) {
        mapHandle = nullptr;
    } else {
        if ((mapHandle = CreateFileMappingW(file->_file_handle, NULL, protect, maximumSizeHigh, maximumSizeLow, nullptr)) == nullptr) {
            throw io_error("{}: Could not create file mapping. '{}'", location(), get_last_error_message());
        }
    }
}

file_mapping::file_mapping(URL const &location, access_mode accessMode, size_t size) :
    file_mapping(findOrOpenFile(location, accessMode), size) {}

file_mapping::~file_mapping()
{
    if (!CloseHandle(mapHandle)) {
        tt_log_error("Could not close file mapping object on file '{}'", get_last_error_message());
    }
}

}
