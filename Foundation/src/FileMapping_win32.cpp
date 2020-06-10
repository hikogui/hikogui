// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/FileMapping.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/memory.hpp"
#include "TTauri/Foundation/required.hpp"
#include <mutex>
#include <Windows.h>

namespace TTauri {

FileMapping::FileMapping(std::shared_ptr<File> const& file, size_t size) :
    file(file), size(size > 0 ? size : File::fileSize(file->location))
{
    DWORD protect;
    if (accessMode() >= (AccessMode::Read | AccessMode::Write)) {
        protect = PAGE_READWRITE;
    }
    else if (accessMode() >= AccessMode::Read) {
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
        if ((mapHandle = CreateFileMappingA(file->fileHandle, NULL, protect, maximumSizeHigh, maximumSizeLow, nullptr)) == nullptr) {
            TTAURI_THROW(io_error("Could not create file mapping")
                .set<error_message_tag>(getLastErrorMessage())
                .set<url_tag>(location())
            );
        }
    }
}

FileMapping::FileMapping(URL const &location, AccessMode accessMode, size_t size) :
    FileMapping(findOrOpenFile(location, accessMode), size) {}

FileMapping::~FileMapping()
{
    if (!CloseHandle(mapHandle)) {
        LOG_ERROR("Could not close file mapping object on file '{}'", getLastErrorMessage());
    }
}

}
