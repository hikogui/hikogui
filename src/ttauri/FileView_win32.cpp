// Copyright 2019 Pokitec
// All rights reserved.

#include "FileView.hpp"
#include "exceptions.hpp"
#include "logger.hpp"
#include "memory.hpp"
#include "URL.hpp"
#include "required.hpp"
#include <mutex>
#include <Windows.h>

namespace tt {

FileView::FileView(std::shared_ptr<FileMapping> const& fileMappingObject, size_t offset, size_t size) :
    fileMappingObject(fileMappingObject),
    _offset(offset)
{
    if (size == 0) {
        size = fileMappingObject->size - _offset;
    }
    tt_assert(_offset + size <= fileMappingObject->size);

    DWORD desiredAccess;
    if (accessMode() >= (access_mode::read | access_mode::write)) {
        desiredAccess = FILE_MAP_WRITE;
    }
    else if (accessMode() >= access_mode::read) {
        desiredAccess = FILE_MAP_READ;
    }
    else {
        TTAURI_THROW(io_error("Illegal access mode WRONLY/0 when viewing file.")
            .set<url_tag>(location())
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
                .set<error_message_tag>(getLastErrorMessage())
                .set<url_tag>(location())
            );
        }
    }

    auto *bytes_ptr = new std::span<std::byte>(static_cast<std::byte *>(data), size);
    _bytes = std::shared_ptr<std::span<std::byte>>(bytes_ptr, FileView::unmap);
}

FileView::FileView(URL const &location, access_mode accessMode, size_t offset, size_t size) :
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

void FileView::unmap(std::span<std::byte> *bytes) noexcept
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
            .set<error_message_tag>(getLastErrorMessage())
            .set<url_tag>(location())
        );
    }
}

}
