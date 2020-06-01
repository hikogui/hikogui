// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/FileView.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/memory.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/required.hpp"
#include <mutex>
#include <Windows.h>

namespace TTauri {

FileView::FileView(std::shared_ptr<FileMapping> const& fileMappingObject, size_t offset, size_t size) :
    fileMappingObject(fileMappingObject),
    _offset(offset)
{
    if (size == 0) {
        size = fileMappingObject->size - _offset;
    }
    ttauri_assert(_offset + size <= fileMappingObject->size);

    DWORD desiredAccess;
    if (accessMode() >= (AccessMode::Read | AccessMode::Write)) {
        desiredAccess = FILE_MAP_WRITE;
    }
    else if (accessMode() >= AccessMode::Read) {
        desiredAccess = FILE_MAP_READ;
    }
    else {
        TTAURI_THROW(io_error("Illegal access mode WRONLY/0 when viewing file.")
            .set<"url"_tag>(location())
        );
    }

    DWORD fileOffsetHigh = _offset >> 32;
    DWORD fileOffsetLow = _offset & 0xffffffff;

    void *data;
    if (size == 0) {
        data = nullptr;
    } else {
        if ((data = MapViewOfFile(fileMappingObject->mapHandle, desiredAccess, fileOffsetHigh, fileOffsetLow, size)) == NULL) {
            TTAURI_THROW(io_error("Could not map view of file.")
                .set<"error_msg"_tag>(getLastErrorMessage())
                .set<"url"_tag>(location())
            );
        }
    }

    auto *bytes_ptr = new nonstd::span<std::byte>(static_cast<std::byte *>(data), size);
    _bytes = std::shared_ptr<nonstd::span<std::byte>>(bytes_ptr, FileView::unmap);
}

FileView::FileView(URL const &location, AccessMode accessMode, size_t offset, size_t size) :
    FileView(findOrCreateFileMappingObject(location, accessMode, offset + size), offset, size) {}

FileView::FileView(FileView const &other) noexcept:
    fileMappingObject(other.fileMappingObject),
    _bytes(other._bytes),
    _offset(other._offset) {}

FileView &FileView::operator=(FileView const &other) noexcept
{
    if (this != &other) {
        fileMappingObject = other.fileMappingObject;
        _offset = other._offset;
        _bytes = other._bytes;
    }
    return *this;
}

FileView::FileView(FileView &&other) noexcept:
    fileMappingObject(std::move(other.fileMappingObject)),
    _bytes(std::move(other._bytes)),
    _offset(other._offset) {}

FileView &FileView::operator=(FileView &&other) noexcept
{
    if (this != &other) {
        fileMappingObject = std::move(other.fileMappingObject);
        _offset = other._offset;
        _bytes = std::move(other._bytes);
    }
    return *this;
}

void FileView::unmap(nonstd::span<std::byte> *bytes) noexcept
{
    if (bytes != nullptr) {
        if (bytes->size() > 0) {
            void *data = bytes->data();
            if (!UnmapViewOfFile(data)) {
                LOG_ERROR("Could not unmap view on file '{}'", getLastErrorMessage());
            }
        }
        delete bytes;
    }
}

void FileView::flush(void* base, size_t size)
{
    if (!FlushViewOfFile(base, size)) {
        TTAURI_THROW(io_error("Could not flush file")
            .set<"error_msg"_tag>(getLastErrorMessage())
            .set<"url"_tag>(location())
        );
    }
}

}
