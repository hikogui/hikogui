// Copyright Jhalak Patel 2021.
// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"
#include "../win32_headers.hpp"


#include <format>
#include <span>
#include <cstddef>
#include <chrono>

export module hikogui_file_file_view_impl;
import hikogui_char_maps; // XXX #616
import hikogui_container;
import hikogui_file_access_mode;
import hikogui_file_file;
import hikogui_file_file_impl;
import hikogui_telemetry;
import hikogui_utility;

export namespace hi { inline namespace v1 {
namespace detail {

class file_view_impl {
public:
    file_view_impl(file_view_impl const&) = delete;
    file_view_impl(file_view_impl&&) = delete;
    file_view_impl& operator=(file_view_impl const&) = delete;
    file_view_impl& operator=(file_view_impl&&) = delete;

    ~file_view_impl()
    {
        if (_data != nullptr) {
            destroy_view(_data);
        }
        if (_mapping_handle) {
            destroy_mapping(_mapping_handle);
        }
    }

    file_view_impl(std::shared_ptr<file_impl> file, std::size_t offset, std::size_t size) :
        _file(std::move(file)), _offset(offset), _size(size)
    {
        if (_size == 0) {
            _size = _file->size() - _offset;
        }

        if (_offset + _size > _file->size()) {
            throw io_error("Requested mapping is beyond file size.");
        }

        if (_file->size() == 0) {
            // Don't map a zero byte file.
            _data = nullptr;

        } else {
            _mapping_handle = make_mapping(file_handle(), access_mode(), _offset + _size);
            try {
                _data = make_view(_mapping_handle, access_mode(), _offset, _size);
            } catch (...) {
                destroy_mapping(_mapping_handle);
                throw;
            }
        }
    }

    [[nodiscard]] std::size_t offset() const noexcept
    {
        return _offset;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return _size;
    }

    [[nodiscard]] hi::access_mode access_mode() const noexcept
    {
        hi_assert_not_null(_file);
        return _file->access_mode();
    }

    [[nodiscard]] void_span void_span() const noexcept
    {
        hi_assert_not_null(_file);
        hi_assert(to_bool(_file->access_mode() & access_mode::write));
        return {_data, _size};
    }

    [[nodiscard]] const_void_span const_void_span() const noexcept
    {
        return {_data, _size};
    }

    [[nodiscard]] bool unmapped() const noexcept
    {
        if (_file != nullptr) {
            if (_file->closed()) {
                _file = nullptr;
                return true;
            } else {
                return false;
            }
        } else {
            return true;
        }
    }

    void unmap()
    {
        if (_data) {
            destroy_view(_data);
            _data = nullptr;
            _size = 0;
            destroy_mapping(_mapping_handle);
            _mapping_handle = nullptr;
        }
        _file = nullptr;
    }

    void flush(hi::void_span span) const noexcept
    {
        if (not FlushViewOfFile(span.data(), span.size())) {
            hi_log_error_once("file::error::flush-view", "Could not flush file. '{}'", get_last_error_message());
        }
    }

private:
    mutable HANDLE _mapping_handle = nullptr;
    mutable std::shared_ptr<file_impl> _file;
    std::size_t _offset;
    std::size_t _size;
    void *_data = nullptr;

    [[nodiscard]] HANDLE file_handle() noexcept
    {
        hi_assert_not_null(_file);
        return _file->file_handle();
    }

    static void destroy_mapping(HANDLE mapping)
    {
        if (not CloseHandle(mapping)) {
            throw io_error(std::format("Could not close file mapping object on file '{}'", get_last_error_message()));
        }
    }

    [[nodiscard]] static HANDLE make_mapping(HANDLE file, hi::access_mode access_mode, std::size_t size)
    {
        hi_assert(size != 0);

        DWORD protect;
        if (to_bool(access_mode & hi::access_mode::read) and to_bool(access_mode & hi::access_mode::write)) {
            protect = PAGE_READWRITE;
        } else if (to_bool(access_mode & hi::access_mode::read)) {
            protect = PAGE_READONLY;
        } else {
            throw io_error("Illegal access mode when mapping file.");
        }

        DWORD size_high = size >> 32;
        DWORD size_low = size & 0xffffffff;

        if (auto r = CreateFileMappingW(file, NULL, protect, size_high, size_low, nullptr)) {
            return r;
        } else {
            throw io_error(std::format("Could not create file mapping. '{}'", get_last_error_message()));
        }
    }

    static void destroy_view(void *data)
    {
        if (not UnmapViewOfFile(data)) {
            throw io_error(std::format("Could not unmap view on file '{}'", get_last_error_message()));
        }
    }

    [[nodiscard]] static void *make_view(HANDLE mapping, hi::access_mode access_mode, std::size_t offset, std::size_t size)
    {
        hi_assert(size != 0);

        DWORD desired_access;
        if (to_bool(access_mode & access_mode::read) and to_bool(access_mode & access_mode::write)) {
            desired_access = FILE_MAP_WRITE;
        } else if (to_bool(access_mode & access_mode::read)) {
            desired_access = FILE_MAP_READ;
        } else {
            throw io_error(std::format("Illegal access mode when viewing file."));
        }

        DWORD offset_high = offset >> 32;
        DWORD offset_low = offset & 0xffffffff;

        if (auto r = MapViewOfFile(mapping, desired_access, offset_high, offset_low, size)) {
            return r;
        } else {
            throw io_error(std::format("Could not map view of file. '{}'", get_last_error_message()));
        }
    }
};

} // namespace detail
}} // namespace hi::v1
