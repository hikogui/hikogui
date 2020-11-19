// Copyright 2019 Pokitec
// All rights reserved.

#include "FileMapping.hpp"
#include "exceptions.hpp"
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
        TTAURI_THROW(io_error("Illigal access mode WRONLY/0 when mapping file.")
            .set<url_tag>(location())
        );
    }

    DWORD maximumSizeHigh = this->size >> 32;
    DWORD maximumSizeLow = this->size & 0xffffffff;

    if (this->size == 0) {
        mapHandle = nullptr;
    } else {
        if ((mapHandle = CreateFileMappingA(file->_file_handle, NULL, protect, maximumSizeHigh, maximumSizeLow, nullptr)) == nullptr) {
            TTAURI_THROW(io_error("Could not create file mapping")
                .set<error_message_tag>(getLastErrorMessage())
                .set<url_tag>(location())
            );
        }
    }
}

FileMapping::FileMapping(URL const &location, access_mode accessMode, size_t size) :
    FileMapping(findOrOpenFile(location, accessMode), size) {}

FileMapping::~FileMapping()
{
    if (!CloseHandle(mapHandle)) {
        LOG_ERROR("Could not close file mapping object on file '{}'", getLastErrorMessage());
    }
}

}
