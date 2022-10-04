// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file file/file_view.hpp Defines the file_view class.
 * @ingroup file
 */

#pragma once

#include "file.hpp"
#include "URL.hpp"
#include "../void_span.hpp"
#include <span>
#include <filesystem>
#include <memory>

hi_warning_push();
// C26490: Don't use reinterpret_cast (type.1).
// We need to convert bytes to chars to get a string_view from the byte buffer.
hi_warning_ignore_msvc(26490);

namespace hi { inline namespace v1 {

namespace detail {

class file_view_impl {
public:
    file_view_impl() = delete;
    file_view_impl(file_view_impl const&) = delete;
    file_view_impl(file_view_impl&&) = delete;
    file_view_impl& operator=(file_view_impl const&) = delete;
    file_view_impl& operator=(file_view_impl&&) = delete;

    virtual ~file_view_impl() = default;
    file_view_impl(std::shared_ptr<file_impl> file, std::size_t offset, std::size_t size) :
        _file(std::move(file)), _offset(offset), _size(size)
    {
        if (_size == 0) {
            _size = _file->size() - _offset;
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
        hi_axiom(_file);
        return _file->access_mode();
    }

    [[nodiscard]] void_span void_span() const noexcept
    {
        hi_axiom(_file);
        hi_assert(to_bool(_file->access_mode() & access_mode::write));
        return {_data, _size};
    }

    [[nodiscard]] const_void_span const_void_span() const noexcept
    {
        return {_data, _size};
    }

    [[nodiscard]] virtual bool unmapped() const noexcept = 0;
    virtual void flush(hi::void_span span) const noexcept = 0;
    virtual void unmap() = 0;

protected:
    mutable std::shared_ptr<file_impl> _file;
    std::size_t _offset;
    std::size_t _size;
    void *_data = nullptr;
};

} // namespace detail

/** Map a file into virtual memory.
 * @ingroup file
 *
 * To map a file into memory there are three objects needed:
 * - The `file` object which holds a handle or file descriptor to an open file on disk.
 * - The `file_mapping` object maps a section of the file in the operating system.
 * - The `file_view` object maps a section of the file-mapping into virtual memory.
 *
 * The `file_mapping` intermediate object is required on Windows systems which
 * holds a handle to a file mapping object.
 *
 */
class file_view {
public:
    ~file_view() = default;
    constexpr file_view() noexcept = default;
    constexpr file_view(file_view const& other) noexcept = default;
    constexpr file_view(file_view&& other) noexcept = default;
    constexpr file_view& operator=(file_view const& other) noexcept = default;
    constexpr file_view& operator=(file_view&& other) noexcept = default;

    /** Create a file-view from a file-mapping.
     *
     * @note The mapping object will be retained by this file-view.
     * @param file An open file object.
     * @param offset The offset from the beginning of the file-mapping (a file mapping may have an offset on its own).
     *               The offset must also be a multiple of the granularity.
     * @param size The size of the mapping, if zero the full file-mapping object is mapped.
     */
    file_view(file const& file, std::size_t offset = 0, std::size_t size = 0);

    file_view(
        std::filesystem::path const& path,
        access_mode access_mode = access_mode::open_for_read,
        std::size_t offset = 0,
        std::size_t size = 0) :
        file_view(file{path, access_mode}, offset, size)
    {
    }

    [[nodiscard]] std::size_t offset() const noexcept
    {
        hi_axiom(_pimpl != nullptr);
        return _pimpl->offset();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        hi_axiom(_pimpl != nullptr);
        return _pimpl->size();
    }

    /** Check if this file view is closed.
     *
     * @post Resources may be released if the file view is closed.
     * @return true of the file view is closed.
     */
    [[nodiscard]] bool unmapped() const noexcept
    {
        if (_pimpl) {
            if (_pimpl->unmapped()) {
                _pimpl = nullptr;
                return true;
            } else {
                return false;
            }
        } else {
            return true;
        }
    }

    /** Check if this file view is open.
     *
     * @post Resources may be released if the file view is closed.
     * @return true of the file view is open.
     */
    explicit operator bool() const noexcept
    {
        return not unmapped();
    }

    void unmap() noexcept
    {
        if (auto pimpl = std::exchange(_pimpl, nullptr)) {
            pimpl->unmap();
        }
    }

    /** Span to the mapping into memory.
     */
    [[nodiscard]] void_span void_span() const noexcept
    {
        hi_axiom(_pimpl != nullptr);
        return _pimpl->void_span();
    }

    /*! Span to the mapping into memory.
     */
    [[nodiscard]] const_void_span const_void_span() const noexcept
    {
        hi_axiom(_pimpl != nullptr);
        return _pimpl->const_void_span();
    }

    /** Flush changes in memory to the open file view.
     *
     * @param span The part of the buffer to flush.
     */
    void flush(hi::void_span span) const noexcept
    {
        hi_axiom(_pimpl != nullptr);
        return _pimpl->flush(span);
    }

    template<typename T>
    [[nodiscard]] friend std::span<T> as_span(file_view const& view) noexcept
    {
        if constexpr (std::is_const_v<T>) {
            return as_span<T>(view.const_void_span());
        } else {
            return as_span<T>(view.void_span());
        }
    }

    [[nodiscard]] friend std::string_view as_string_view(file_view const& view) noexcept
    {
        hi_axiom(view.offset() == 0);
        return as_string_view(view.const_void_span());
    }

    [[nodiscard]] friend bstring_view as_bstring_view(file_view const& view) noexcept
    {
        hi_axiom(view.offset() == 0);
        return as_bstring_view(view.const_void_span());
    }

private:
    mutable std::shared_ptr<detail::file_view_impl> _pimpl;
};

}} // namespace hi::v1

hi_warning_pop();
