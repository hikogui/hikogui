// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/FileView.hpp"
#include "TTauri/Diagnostic/exceptions.hpp"
#include "TTauri/Diagnostic/logger.hpp"
#include "TTauri/Required/memory.hpp"
#include "TTauri/Required/URL.hpp"
#include "TTauri/Required/required.hpp"
#include <mutex>

#if OPERATING_SYSTEM == OS_WINDOWS
#include <Windows.h>
#endif

namespace TTauri {

FileView::FileView(std::shared_ptr<FileMapping> const& fileMappingObject, size_t offset, size_t size) :
    fileMappingObject(fileMappingObject),
    _offset(offset)
{
    if (size == 0) {
        size = fileMappingObject->size - _offset;
    }
    required_assert(_offset + size <= fileMappingObject->size);

#if OPERATING_SYSTEM == OS_WINDOWS
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
    if ((data = MapViewOfFile(fileMappingObject->mapHandle, desiredAccess, fileOffsetHigh, fileOffsetLow, size)) == NULL) {
        TTAURI_THROW(io_error("Could not map view of file.")
            .set<"error_message"_tag>(getLastErrorMessage())
            .set<"url"_tag>(location())
        );
    }

    auto *bytes_ptr = new gsl::span<std::byte>(static_cast<std::byte *>(data), size);
    _bytes = std::shared_ptr<gsl::span<std::byte>>(bytes_ptr, FileView::unmap);
#endif
}

FileView::FileView(URL const &location, AccessMode accessMode, size_t offset, size_t size) :
    FileView(findOrCreateFileMappingObject(location, accessMode, offset + size), offset, size) {}

FileView::FileView(FileView const &other) noexcept:
    fileMappingObject(other.fileMappingObject),
    _bytes(other._bytes),
    _offset(other._offset)
{
}

FileView &FileView::operator=(FileView const &other) noexcept
{
    fileMappingObject = other.fileMappingObject;
    _offset = other._offset;
    _bytes = other._bytes;
    return *this;
}

FileView::FileView(FileView &&other) noexcept:
    fileMappingObject(std::move(other.fileMappingObject)),
    _bytes(std::move(other._bytes)),
    _offset(other._offset)
{
}

FileView &FileView::operator=(FileView &&other) noexcept
{
    fileMappingObject = std::move(other.fileMappingObject);
    _offset = other._offset;
    _bytes = std::move(other._bytes);
    return *this;
}

void FileView::unmap(gsl::span<std::byte> *bytes) noexcept
{
    if (bytes != nullptr) {
        if (bytes->size() > 0) {
#if OPERATING_SYSTEM == OS_WINDOWS
            void *data = bytes->data();
            if (!UnmapViewOfFile(data)) {
                LOG_ERROR("Could not unmap view on file '{}'", getLastErrorMessage());
            }
#endif
        }
        delete bytes;
    }
}

void FileView::flush(void* base, size_t size)
{
#if OPERATING_SYSTEM == OS_WINDOWS
    if (!FlushViewOfFile(base, size)) {
        TTAURI_THROW(io_error("Could not flush file")
            .set<"error_message"_tag>(getLastErrorMessage())
            .set<"url"_tag>(location())
        );
    }
#endif
}

std::shared_ptr<FileMapping> FileView::findOrCreateFileMappingObject(URL const& location, AccessMode accessMode, size_t size)
{
    static std::mutex mutex;
    static std::unordered_map<URL, std::vector<std::weak_ptr<FileMapping>>> mappedFileObjects;

    let lock = std::scoped_lock(mutex);

    cleanupWeakPointers(mappedFileObjects);

    auto& mappings = mappedFileObjects[location];

    for (auto weak_fileMappingObject : mappings) {
        if (auto fileMappingObject = weak_fileMappingObject.lock()) {
            if (fileMappingObject->size >= size && fileMappingObject->accessMode() >= accessMode) {
                return fileMappingObject;
            }
        }
    }

    auto fileMappingObject = std::make_shared<FileMapping>(location, accessMode, size);
    mappings.push_back(fileMappingObject);
    return fileMappingObject;
}

}
