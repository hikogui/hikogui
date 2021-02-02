// Copyright 2019 Pokitec
// All rights reserved.

#include "FileMapping.hpp"
#include "exception.hpp"
#include "error_info.hpp"
#include "logger.hpp"
#include "memory.hpp"
#include "required.hpp"
#include <mutex>
#include <Windows.h>

namespace tt {

FileMapping::FileMapping(std::shared_ptr<tt::file> const &file, size_t size) :
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
        tt_error_info().set<url_tag>(location());
        throw io_error("Illegal access mode WRONLY/0 when mapping file.");
    }

    DWORD maximumSizeHigh = this->size >> 32;
    DWORD maximumSizeLow = this->size & 0xffffffff;

    if (this->size == 0) {
        mapHandle = nullptr;
    } else {
        if ((mapHandle = CreateFileMappingA(file->_file_handle, NULL, protect, maximumSizeHigh, maximumSizeLow, nullptr)) == nullptr) {
            tt_error_info().set<error_message_tag>(getLastErrorMessage()).set<url_tag>(location());
            throw io_error("Could not create file mapping");
        }
    }
}

FileMapping::FileMapping(URL const &location, access_mode accessMode, size_t size) :
    FileMapping(findOrOpenFile(location, accessMode), size) {}

FileMapping::~FileMapping()
{
    if (!CloseHandle(mapHandle)) {
        tt_log_error("Could not close file mapping object on file '{}'", getLastErrorMessage());
    }
}

}
