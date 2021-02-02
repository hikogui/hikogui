// Copyright 2019 Pokitec
// All rights reserved.

#include "FileView.hpp"
#include "exception.hpp"
#include "logger.hpp"
#include "memory.hpp"
#include "URL.hpp"
#include "required.hpp"
#include <mutex>
#include <sys/mman.h>

namespace tt {

FileView::FileView(std::shared_ptr<FileMapping> const &fileMappingObject, size_t offset, size_t size) :
    fileMappingObject(fileMappingObject), _offset(offset)
{
    if (size == 0) {
        size = fileMappingObject->size - _offset;
    }
    tt_assert(_offset + size <= fileMappingObject->size);

    int prot;
    if (accessMode() >= (AccessMode::Read | AccessMode::Write)) {
        prot = PROT_WRITE | PROT_READ;
    } else if (accessMode() >= AccessMode::Read) {
        prot = PROT_READ;
    } else {
        tt_error_info().set<url_tag>(location());
        throw io_error("Illegal access mode write-only when viewing file.");
    }

    int flags = MAP_SHARED;

    void *data;
    if ((data = ::mmap(0, size, prot, flags, fileMappingObject->file->fileHandle, _offset)) == MAP_FAILED) {
        tt_error_info().set<error_message_tag>(getLastErrorMessage()).set<url_tag>(location());
        throw io_error("Could not map view of file.");
    }

    auto *bytes_ptr = new std::span<std::byte>(static_cast<std::byte *>(data), size);
    _bytes = std::shared_ptr<std::span<std::byte>>(bytes_ptr, FileView::unmap);
}

FileView::FileView(URL const &location, AccessMode accessMode, size_t offset, size_t size) :
    FileView(findOrCreateFileMappingObject(location, accessMode, offset + size), offset, size)
{
}

FileView::FileView(FileView const &other) noexcept :
    fileMappingObject(other.fileMappingObject), _bytes(other._bytes), _offset(other._offset)
{
}

FileView &FileView::operator=(FileView const &other) noexcept
{
    if (this != &other) {
        fileMappingObject = other.fileMappingObject;
        _offset = other._offset;
        _bytes = other._bytes;
    }
    return *this;
}

FileView::FileView(FileView &&other) noexcept :
    fileMappingObject(std::move(other.fileMappingObject)), _bytes(std::move(other._bytes)), _offset(other._offset)
{
}

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
        if (!bytes->empty()) {
            if (!munmap(bytes->data(), bytes->size())) {
                tt_log_error("Could not munmap view on file '{}'", getLastErrorMessage());
            }
        }
        delete bytes;
    }
}

void FileView::flush(void *base, size_t size)
{
    int flags = MS_SYNC;
    if (!msync(base, size, flags)) {
        tt_error_info().set<error_message_tag>(getLastErrorMessage()).set<url_tag>(location());
        throw io_error("Could not flush file");
    }
}

} // namespace tt
