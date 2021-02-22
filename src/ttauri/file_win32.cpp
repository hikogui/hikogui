// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "file.hpp"
#include "logger.hpp"
#include "exception.hpp"
#include "strings.hpp"
#include "cast.hpp"
#include <type_traits>
#include <Windows.h>

namespace tt {

file::file(URL const &location, access_mode access_mode) : _access_mode(access_mode), _location(location)
{
    DWORD desiredAccess = 0;
    if (_access_mode >= (access_mode::read | access_mode::write)) {
        desiredAccess = GENERIC_READ | GENERIC_WRITE;
    } else if (_access_mode >= access_mode::read) {
        desiredAccess = GENERIC_READ;
    } else if (_access_mode >= access_mode::write) {
        desiredAccess = GENERIC_WRITE;
    } else {
        throw io_error("{}: Invalid AccessMode; expecting Readable and/or Writeable.", location);
    }

    DWORD shareMode;
    if (_access_mode >= access_mode::write_lock) {
        shareMode = 0;
    } else if (_access_mode >= access_mode::read_lock) {
        shareMode = FILE_SHARE_READ;
    } else {
        // Allow files to be renamed and deleted.
        shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    }

    DWORD creationDisposition;
    if (_access_mode >= (access_mode::create | access_mode::open)) {
        if (_access_mode >= access_mode::truncate) {
            creationDisposition = CREATE_ALWAYS;
        } else {
            creationDisposition = OPEN_ALWAYS;
        }

    } else if (_access_mode >= access_mode::create) {
        creationDisposition = CREATE_NEW;

    } else if (_access_mode >= access_mode::open) {
        if (_access_mode >= access_mode::truncate) {
            creationDisposition = TRUNCATE_EXISTING;
        } else {
            creationDisposition = OPEN_EXISTING;
        }

    } else {
        throw io_error("{}: Invalid AccessMode; expecting CreateFile and/or OpenFile.", location);
    }

    DWORD flagsAndAttributes = 0;
    if (_access_mode >= access_mode::random) {
        flagsAndAttributes |= FILE_FLAG_RANDOM_ACCESS;
    }
    if (_access_mode >= access_mode::sequential) {
        flagsAndAttributes |= FILE_FLAG_SEQUENTIAL_SCAN;
    }
    if (_access_mode >= access_mode::write_through) {
        flagsAndAttributes |= FILE_FLAG_WRITE_THROUGH;
    }

    if (_access_mode >= access_mode::rename) {
        desiredAccess |= DELETE;
    }

    ttlet fileName = _location.nativeWPath();
    if ((_file_handle =
             CreateFileW(fileName.data(), desiredAccess, shareMode, NULL, creationDisposition, flagsAndAttributes, NULL)) !=
        INVALID_HANDLE_VALUE) {
        return;
    }

    ttlet error = GetLastError();
    if (_access_mode >= access_mode::create_directories && error == ERROR_PATH_NOT_FOUND &&
        (creationDisposition == CREATE_ALWAYS || creationDisposition == OPEN_ALWAYS || creationDisposition == CREATE_NEW)) {
        // Retry opening the file, by first creating the directory hierarchy.
        ttlet directory = _location.urlByRemovingFilename();
        file::create_directory_hierarchy(directory);

        if ((_file_handle =
                 CreateFileW(fileName.data(), desiredAccess, shareMode, NULL, creationDisposition, flagsAndAttributes, NULL)) !=
            INVALID_HANDLE_VALUE) {
            return;
        }
    }
    throw io_error("{}: Could not open file, '{}'", _location, get_last_error_message());
}

file::~file() noexcept
{
    close();
}

void file::flush()
{
    tt_axiom(_file_handle);

    if (!FlushFileBuffers(_file_handle)) {
        throw io_error("{}: Could not flush file. '{}'", _location, get_last_error_message());
    }
}

void file::close()
{
    if (_file_handle != INVALID_HANDLE_VALUE) {
        if (!CloseHandle(_file_handle)) {
            throw io_error("{}: Could not close file. '{}'", _location, get_last_error_message());
        }
        _file_handle = INVALID_HANDLE_VALUE;
    }
}

size_t file::size() const
{
    BY_HANDLE_FILE_INFORMATION file_information;

    if (!GetFileInformationByHandle(_file_handle, &file_information)) {
        throw io_error("{}: Could not get file information. '{}'", _location, get_last_error_message());
    }

    return merge_bit_cast<size_t>(file_information.nFileSizeHigh, file_information.nFileSizeLow);
}

ssize_t file::seek(ssize_t offset, seek_whence whence)
{
    tt_axiom(_file_handle);

    DWORD whence_;
    switch (whence) {
        using enum seek_whence;
    case begin: whence_ = FILE_BEGIN; break;
    case current: whence_ = FILE_CURRENT; break;
    case end: whence_ = FILE_END; break;
    default: tt_no_default();
    }

    LONG offset_lo = low_bit_cast<LONG>(offset);
    LONG offset_hi = high_bit_cast<LONG>(offset);

    offset_lo = SetFilePointer(_file_handle, offset_lo, &offset_hi, whence_);

    if (offset_lo == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
        throw io_error("{}: Could not seek in file. '{}'", _location, get_last_error_message());
    }

    return merge_bit_cast<ssize_t>(offset_hi, offset_lo);
}

void file::rename(URL const &destination, bool overwrite_existing)
{
    auto dst_filename = destination.nativeWPath();
    auto dst_filename_wsize = (std::size(dst_filename) + 1) *
        sizeof(WCHAR);

    ttlet rename_info_size = narrow_cast<DWORD>(sizeof(_FILE_RENAME_INFO) + dst_filename_wsize);

    auto rename_info = reinterpret_cast<PFILE_RENAME_INFO>(std::malloc(rename_info_size));
    if (rename_info == nullptr) {
        throw std::bad_alloc();
    }

    rename_info->ReplaceIfExists = overwrite_existing;
    rename_info->RootDirectory = nullptr;
    rename_info->FileNameLength = narrow_cast<DWORD>(dst_filename_wsize);
    std::memcpy(rename_info->FileName, dst_filename.c_str(), dst_filename_wsize);

    ttlet r = SetFileInformationByHandle(_file_handle, FileRenameInfo, rename_info, rename_info_size);
    free(rename_info);

    if (!r) {
        throw io_error("{}: Could not rename file to {}. '{}'", _location, destination, get_last_error_message());
    }
}

/*! Write data to a file.
 */
ssize_t file::write(std::byte const *data, ssize_t size, ssize_t offset)
{
    tt_axiom(size >= 0);
    tt_axiom(_file_handle != INVALID_HANDLE_VALUE);

    ssize_t total_written_size = 0;
    while (size) {
        ttlet to_write_size = static_cast<DWORD>(std::min(size, static_cast<ssize_t>(std::numeric_limits<DWORD>::max())));
        DWORD written_size = 0;

        OVERLAPPED overlapped;
        std::memset(&overlapped, 0, sizeof(OVERLAPPED));
        overlapped.Offset = low_bit_cast<DWORD>(offset);
        overlapped.OffsetHigh = high_bit_cast<DWORD>(offset);
        auto overlapped_ptr = offset != -1 ? &overlapped : nullptr;

        if (!WriteFile(_file_handle, data, to_write_size, &written_size, overlapped_ptr)) {
            throw io_error("{}: Could not write to file. '{}'", _location, get_last_error_message());
        } else if (written_size == 0) {
            break;
        }

        data += written_size;
        if (offset != -1) {
            offset += written_size;
        }
        size -= written_size;
        total_written_size += written_size;
    }

    return total_written_size;
}

ssize_t file::read(std::byte *data, ssize_t size, ssize_t offset)
{
    tt_axiom(size >= 0);
    tt_axiom(_file_handle != INVALID_HANDLE_VALUE);

    ssize_t total_read_size = 0;
    while (size) {
        ttlet to_read_size = static_cast<DWORD>(std::min(size, static_cast<ssize_t>(std::numeric_limits<DWORD>::max())));
        DWORD read_size = 0;

        OVERLAPPED overlapped;
        std::memset(&overlapped, 0, sizeof(OVERLAPPED));
        overlapped.Offset = low_bit_cast<DWORD>(offset);
        overlapped.OffsetHigh = high_bit_cast<DWORD>(offset);
        auto overlapped_ptr = offset != -1 ? &overlapped : nullptr;

        if (!ReadFile(_file_handle, data, to_read_size, &read_size, overlapped_ptr)) {
            throw io_error("{}: Could not read from file. '{}'", _location, get_last_error_message());
        } else if (read_size == 0) {
            break;
        }

        data += read_size;
        if (offset != -1) {
            offset += read_size;
        }
        size -= read_size;
        total_read_size += read_size;
    }

    return total_read_size;
}

bstring file::read_bstring(ssize_t size, ssize_t offset)
{
    ttlet offset_ = offset == -1 ? get_seek() : offset;
    ttlet size_ = std::ssize(*this) - offset_;

    auto r = bstring{};
    r.resize(size_);
    ttlet bytes_read = read(r.data(), size_, offset_);
    r.resize(bytes_read);

    if (offset == -1) {
        seek(offset_ + bytes_read);
    }
    return r;
}

std::string file::read_string(ssize_t max_size)
{
    ttlet size_ = std::ssize(*this);
    if (size_ > max_size) {
        throw io_error("{}: File size is larger than max_size.", _location);
    }

    auto r = std::string{};
    r.resize(size_);
    ttlet bytes_read = read(r.data(), size_, 0);
    r.resize(bytes_read);
    return r;
}

std::u8string file::read_u8string(ssize_t max_size)
{
    ttlet size_ = std::ssize(*this);
    if (size_ > max_size) {
        throw io_error("{}: File size is larger than max_size.", _location);
    }

    auto r = std::u8string{};
    r.resize(size_);
    ttlet bytes_read = read(r.data(), size_, 0);
    r.resize(bytes_read);
    return r;
}

size_t file::file_size(URL const &url)
{
    ttlet name = url.nativeWPath();

    WIN32_FILE_ATTRIBUTE_DATA attributes;
    if (GetFileAttributesExW(name.data(), GetFileExInfoStandard, &attributes) == 0) {
        throw io_error("{}: Could not retrieve file attributes.", url);
    }

    LARGE_INTEGER size;
    size.HighPart = attributes.nFileSizeHigh;
    size.LowPart = attributes.nFileSizeLow;
    return narrow_cast<int64_t>(size.QuadPart);
}

void file::create_directory(URL const &url, bool hierarchy)
{
    if (url.isRootDirectory()) {
        throw io_error("Cannot create a root directory.");
    }

    ttlet directory_name = url.nativeWPath();
    if (CreateDirectoryW(directory_name.data(), nullptr)) {
        return;
    }

    if (hierarchy && GetLastError() == ERROR_PATH_NOT_FOUND) {
        try {
            file::create_directory(url.urlByRemovingFilename(), true);
        } catch (io_error const &e) {
            throw io_error("{}: Could not create directory, while creating in between directory.\n{}", url, e.what());
        }

        if (CreateDirectoryW(directory_name.data(), nullptr)) {
            return;
        }
    }

    throw io_error("{}: Could not create directory. '{}'", url, get_last_error_message());
}

void file::create_directory_hierarchy(URL const &url)
{
    return create_directory(url, true);
}

} // namespace tt
