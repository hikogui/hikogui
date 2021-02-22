// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "file_view.hpp"
#include "exception.hpp"
#include "logger.hpp"
#include "memory.hpp"
#include "URL.hpp"
#include "required.hpp"
#include <mutex>
#include <sys/mman.h>

namespace tt {

file_view::file_view(std::shared_ptr<file_mapping> const &_file_mapping_object, size_t offset, size_t size) :
    _file_mapping_object(_file_mapping_object), _offset(offset)
{
    if (size == 0) {
        size = _file_mapping_object->size - _offset;
    }
    tt_assert(_offset + size <= _file_mapping_object->size);

    int prot;
    if (accessMode() >= (AccessMode::Read | AccessMode::Write)) {
        prot = PROT_WRITE | PROT_READ;
    } else if (accessMode() >= AccessMode::Read) {
        prot = PROT_READ;
    } else {
        throw io_error("{}: Illegal access mode write-only when viewing file.", location());
    }

    int flags = MAP_SHARED;

    void *data;
    if ((data = ::mmap(0, size, prot, flags, _file_mapping_object->file->fileHandle, _offset)) == MAP_FAILED) {
        throw io_error("{}: Could not map view of file. '{}'", location(), get_last_error_message());
    }

    auto *bytes_ptr = new std::span<std::byte>(static_cast<std::byte *>(data), size);
    _bytes = std::shared_ptr<std::span<std::byte>>(bytes_ptr, file_view::unmap);
}

file_view::file_view(URL const &location, AccessMode accessMode, size_t offset, size_t size) :
    file_view(findOrCreateFileMappingObject(location, accessMode, offset + size), offset, size)
{
}

file_view::file_view(file_view const &other) noexcept :
    _file_mapping_object(other._file_mapping_object), _bytes(other._bytes), _offset(other._offset)
{
}

file_view &file_view::operator=(file_view const &other) noexcept
{
    if (this != &other) {
        _file_mapping_object = other._file_mapping_object;
        _offset = other._offset;
        _bytes = other._bytes;
    }
    return *this;
}

file_view::file_view(file_view &&other) noexcept :
    _file_mapping_object(std::move(other._file_mapping_object)), _bytes(std::move(other._bytes)), _offset(other._offset)
{
}

file_view &file_view::operator=(file_view &&other) noexcept
{
    if (this != &other) {
        _file_mapping_object = std::move(other._file_mapping_object);
        _offset = other._offset;
        _bytes = std::move(other._bytes);
    }
    return *this;
}

void file_view::unmap(std::span<std::byte> *bytes) noexcept
{
    if (bytes != nullptr) {
        if (!bytes->empty()) {
            if (!munmap(bytes->data(), bytes->size())) {
                tt_log_error("Could not munmap view on file '{}'", get_last_error_message());
            }
        }
        delete bytes;
    }
}

void file_view::flush(void *base, size_t size)
{
    int flags = MS_SYNC;
    if (!msync(base, size, flags)) {
        throw io_error("{}: Could not flush file '{}'", location(), get_last_error_message());
    }
}

} // namespace tt
