// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../utility/win32_headers.hpp"

#include "file_win32.hpp"
#include "../log.hpp"
#include "../utility/module.hpp"
#include "../strings.hpp"
#include "../defer.hpp"
#include <type_traits>

namespace hi { inline namespace v1 {
namespace detail {

file_win32::~file_win32() noexcept
{
    close();
}

file_win32::file_win32(std::filesystem::path const& path, hi::access_mode access_mode) : file_impl(access_mode)
{
    DWORD desired_access = 0;
    if (to_bool(access_mode & access_mode::read) and to_bool(access_mode & access_mode::write)) {
        desired_access = GENERIC_READ | GENERIC_WRITE;
    } else if (to_bool(access_mode & access_mode::read)) {
        desired_access = GENERIC_READ;
    } else if (to_bool(access_mode & access_mode::write)) {
        desired_access = GENERIC_WRITE;
    } else {
        throw io_error(std::format("{}: Invalid AccessMode; expecting Readable and/or Writeable.", to_string(path.u8string())));
    }

    DWORD share_mode;
    if (to_bool(access_mode & access_mode::write_lock)) {
        share_mode = 0;
    } else if (to_bool(access_mode & access_mode::read_lock)) {
        share_mode = FILE_SHARE_READ;
    } else {
        // Allow files to be renamed and deleted.
        share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    }

    DWORD creation_disposition;
    if (to_bool(access_mode & access_mode::create) and to_bool(access_mode & access_mode::open)) {
        if (to_bool(access_mode & access_mode::truncate)) {
            creation_disposition = CREATE_ALWAYS;
        } else {
            creation_disposition = OPEN_ALWAYS;
        }

    } else if (to_bool(access_mode & access_mode::create)) {
        creation_disposition = CREATE_NEW;

    } else if (to_bool(access_mode & access_mode::open)) {
        if (to_bool(access_mode & access_mode::truncate)) {
            creation_disposition = TRUNCATE_EXISTING;
        } else {
            creation_disposition = OPEN_EXISTING;
        }

    } else {
        throw io_error(std::format("{}: Invalid AccessMode; expecting CreateFile and/or OpenFile.", to_string(path.u8string())));
    }

    DWORD flags_and_attributes = 0;
    if (to_bool(access_mode & access_mode::random)) {
        flags_and_attributes |= FILE_FLAG_RANDOM_ACCESS;
    }
    if (to_bool(access_mode & access_mode::sequential)) {
        flags_and_attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
    }
    if (to_bool(access_mode & access_mode::write_through)) {
        flags_and_attributes |= FILE_FLAG_WRITE_THROUGH;
    }

    if (to_bool(access_mode & access_mode::rename)) {
        desired_access |= DELETE;
    }

    hilet file_name = path.native();
    if ((_file_handle =
             CreateFileW(file_name.data(), desired_access, share_mode, NULL, creation_disposition, flags_and_attributes, NULL)) !=
        INVALID_HANDLE_VALUE) {
        return;
    }

    hilet error = GetLastError();
    if (to_bool(access_mode & access_mode::create_directories) and error == ERROR_PATH_NOT_FOUND and
        (creation_disposition == CREATE_ALWAYS or creation_disposition == OPEN_ALWAYS or creation_disposition == CREATE_NEW)) {
        // Retry opening the file, by first creating the directory hierarchy.
        auto directory = path;
        directory.remove_filename();
        std::filesystem::create_directories(directory);

        if ((_file_handle = CreateFileW(
                 file_name.data(), desired_access, share_mode, NULL, creation_disposition, flags_and_attributes, NULL)) !=
            INVALID_HANDLE_VALUE) {
            return;
        }
    }
    throw io_error(std::format("{}: Could not open file, '{}'", path.string(), get_last_error_message()));
}

void file_win32::flush()
{
    hi_assert_not_null(_file_handle);

    if (not FlushFileBuffers(_file_handle)) {
        throw io_error(std::format("{}: Could not flush file.", get_last_error_message()));
    }
}

void file_win32::close()
{
    if (_file_handle != INVALID_HANDLE_VALUE) {
        if (!CloseHandle(_file_handle)) {
            throw io_error(std::format("{}: Could not close file.", get_last_error_message()));
        }
        _file_handle = INVALID_HANDLE_VALUE;
    }
}

std::size_t file_win32::size() const
{
    BY_HANDLE_FILE_INFORMATION file_information;

    if (not GetFileInformationByHandle(_file_handle, &file_information)) {
        throw io_error(std::format("{}: Could not get file information.", get_last_error_message()));
    }

    return merge_bit_cast<std::size_t>(file_information.nFileSizeHigh, file_information.nFileSizeLow);
}

std::size_t file_win32::seek(ssize_t offset, seek_whence whence)
{
    hi_assert_not_null(_file_handle);

    DWORD whence_;
    switch (whence) {
        using enum seek_whence;
    case begin:
        whence_ = FILE_BEGIN;
        break;
    case current:
        whence_ = FILE_CURRENT;
        break;
    case end:
        whence_ = FILE_END;
        break;
    default:
        hi_no_default();
    }

    LARGE_INTEGER offset_;
    LARGE_INTEGER new_offset;
    offset_.QuadPart = narrow_cast<LONGLONG>(offset);
    if (not SetFilePointerEx(_file_handle, offset_, &new_offset, whence_)) {
        throw io_error(std::format("{}: Could not seek in file.", get_last_error_message()));
    }

    return narrow_cast<std::size_t>(new_offset.QuadPart);
}

void file_win32::rename(std::filesystem::path const& destination, bool overwrite_existing)
{
    auto dst_filename = destination.native();
    auto dst_filename_wsize = (dst_filename.size() + 1) * sizeof(WCHAR);

    hilet rename_info_size = narrow_cast<DWORD>(sizeof(_FILE_RENAME_INFO) + dst_filename_wsize);

    auto rename_info_alloc = std::string{};
    rename_info_alloc.resize(rename_info_size);

    auto rename_info = reinterpret_cast<PFILE_RENAME_INFO>(rename_info_alloc.data());

    rename_info->ReplaceIfExists = overwrite_existing;
    rename_info->RootDirectory = nullptr;
    rename_info->FileNameLength = narrow_cast<DWORD>(dst_filename_wsize);
    std::memcpy(rename_info->FileName, dst_filename.c_str(), dst_filename_wsize);

    if (not SetFileInformationByHandle(_file_handle, FileRenameInfo, rename_info, rename_info_size)) {
        throw io_error(std::format("Could not rename file to '{}': {}", destination.string(), get_last_error_message()));
    }
}

void file_win32::write(void const *data, std::size_t size)
{
    hi_assert(_file_handle != INVALID_HANDLE_VALUE);

    while (size != 0) {
        // Copy in blocks of 32 kByte
        auto to_write = size < 0x8000 ? narrow_cast<DWORD>(size) : DWORD{0x8000};
        auto written = DWORD{};
        if (not WriteFile(_file_handle, data, to_write, &written, nullptr)) {
            throw io_error(std::format("{}: Could not write to file.", get_last_error_message()));

        } else if (written == 0) {
            throw io_error("Could not write to file. Reached end-of-file.");
        }

        advance_bytes(data, written);
        size -= written;
    }
}

std::size_t file_win32::read(void *data, std::size_t size)
{
    hi_assert(_file_handle != INVALID_HANDLE_VALUE);

    ssize_t total_read = 0;
    while (size) {
        auto to_read = size < 0x8000 ? narrow_cast<DWORD>(size) : DWORD{0x8000};
        auto has_read = DWORD{};

        if (!ReadFile(_file_handle, data, to_read, &has_read, nullptr)) {
            throw io_error(std::format("{}: Could not read from file.", get_last_error_message()));

        } else if (has_read == 0) {
            // Read to end-of-file.
            break;
        }

        advance_bytes(data, has_read);
        size -= has_read;
        total_read += has_read;
    }

    return total_read;
}

} // namespace detail

file::file(std::filesystem::path const& path, hi::access_mode access_mode) :
    _pimpl(std::make_shared<detail::file_win32>(path, access_mode))
{
}

}} // namespace hi::v1
