// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "file_view.hpp"
#include "../utility/module.hpp"
#include "log.hpp"
#include <mutex>
#include <sys/mman.h>

namespace hi::inline v1 {

file_view::file_view(std::shared_ptr<file_mapping> const &_file_mapping_object, std::size_t offset, std::size_t size) :
    _file_mapping_object(_file_mapping_object), _offset(offset)
{
    if (size == 0) {
        size = _file_mapping_object->size - _offset;
    }
    hi_assert(_offset + size <= _file_mapping_object->size);

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

    auto *bytes_ptr = new void_span(data, size);
    _bytes = std::shared_ptr<void_span>(bytes_ptr, file_view::unmap);
}

file_view::file_view(std::filesystem::path const &path, AccessMode accessMode, std::size_t offset, std::size_t size) :
    file_view(findOrCreateFileMappingObject(path, accessMode, offset + size), offset, size)
{
}

file_view::file_view(file_view const &other) noexcept :
    _file_mapping_object(other._file_mapping_object), _bytes(other._bytes), _offset(other._offset)
{
    hi_assert(&other != this);
}

file_view &file_view::operator=(file_view const &other) noexcept
{
    hi_return_on_self_assignment(other);
    _file_mapping_object = other._file_mapping_object;
    _offset = other._offset;
    _bytes = other._bytes;
    return *this;
}

file_view::file_view(file_view &&other) noexcept :
    _file_mapping_object(std::move(other._file_mapping_object)), _bytes(std::move(other._bytes)), _offset(other._offset)
{
    hi_assert(&other != this);
}

file_view &file_view::operator=(file_view &&other) noexcept
{
    hi_return_on_self_assignment(other);
    _file_mapping_object = std::move(other._file_mapping_object);
    _offset = other._offset;
    _bytes = std::move(other._bytes);
    return *this;
}

void file_view::unmap(void_span *bytes) noexcept
{
    if (bytes != nullptr) {
        if (!bytes->empty()) {
            if (!munmap(bytes->data(), bytes->size())) {
                hi_log_error("Could not munmap view on file '{}'", get_last_error_message());
            }
        }
        delete bytes;
    }
}

void file_view::flush(void *base, std::size_t size)
{
    int flags = MS_SYNC;
    if (!msync(base, size, flags)) {
        throw io_error("{}: Could not flush file '{}'", location(), get_last_error_message());
    }
}

} // namespace hi::inline v1
