// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file file/file.hpp Defines the file class.
 * @ingroup file
 */

#pragma once

#include "../byte_string.hpp"
#include "../utility/module.hpp"
#include <mutex>
#include <cstdint>
#include <map>
#include <span>
#include <filesystem>

namespace hi { inline namespace v1 {

/** The position in the file to seek from.
 * @ingroup file
 */
enum class seek_whence {
    begin, ///< Start from the beginning of the file.
    current, ///< Continue from the current position.
    end ///< Start from the end of the file.
};

/** The mode in which way to open a file.
 * @ingroup file
 *
 * These flags can be combined by using OR.
 */
enum class access_mode {
    read = 0x1, ///< Allow read access to a file.
    write = 0x2, ///< Allow write access to a file.
    rename = 0x4, ///< Allow renaming an open file.
    read_lock = 0x10, ///< Lock the file for reading, i.e. shared-lock.
    write_lock = 0x20, ///< Lock the file for writing, i.e. exclusive-lock.
    open = 0x100, ///< Open file if it exist, or fail.
    create = 0x200, ///< Create file if it does not exist, or fail.
    truncate = 0x400, ///< After the file has been opened, truncate it.
    random = 0x1000, ///< Hint the data should not be prefetched.
    sequential = 0x2000, ///< Hint that the data should be prefetched.
    no_reuse = 0x4000, ///< Hint that the data should not be cached.
    write_through = 0x8000, ///< Hint that writes should be send directly to disk.
    create_directories = 0x10000, ///< Create directory hierarchy, if the file could not be created.

    open_for_read = open | read, ///< Default open a file for reading.
    open_for_read_and_write = open | read | write, ///< Default open a file for reading and writing.
    truncate_or_create_for_write = create_directories | open | create | truncate | write
};

[[nodiscard]] constexpr access_mode operator|(access_mode const& lhs, access_mode const& rhs) noexcept
{
    return static_cast<access_mode>(to_underlying(lhs) | to_underlying(rhs));
}

[[nodiscard]] constexpr access_mode operator&(access_mode const& lhs, access_mode const& rhs) noexcept
{
    return static_cast<access_mode>(to_underlying(lhs) & to_underlying(rhs));
}

[[nodiscard]] constexpr bool to_bool(access_mode const& rhs) noexcept
{
    return to_bool(to_underlying(rhs));
}

namespace detail {

class file_impl {
public:
    virtual ~file_impl() = default;
    file_impl(file_impl const& other) = delete;
    file_impl(file_impl&& other) = delete;
    file_impl& operator=(file_impl const& other) = delete;
    file_impl& operator=(file_impl&& other) = delete;

    file_impl(access_mode access_mode) : _access_mode(access_mode) {}

    [[nodiscard]] hi::access_mode access_mode() const noexcept
    {
        return _access_mode;
    }

    [[nodiscard]] virtual bool closed() = 0;
    virtual void close() = 0;
    virtual void flush() = 0;
    virtual void rename(std::filesystem::path const& destination, bool overwrite_existing) = 0;
    [[nodiscard]] virtual std::size_t size() const = 0;
    [[nodiscard]] virtual std::size_t seek(std::ptrdiff_t offset, seek_whence whence) = 0;
    virtual void write(void const *data, std::size_t size) = 0;
    [[nodiscard]] virtual std::size_t read(void *data, std::size_t size) = 0;

protected:
    hi::access_mode _access_mode;
};

} // namespace detail

/** A File object.
 * @ingroup file
 */
class file {
public:
    /** Open a file at location.
     * @param path The path to the file to open.
     * @param access_mode access-mode to open the file.
     */
    file(std::filesystem::path const& path, access_mode access_mode = access_mode::open_for_read);

    ~file() = default;
    file(file const& other) noexcept = default;
    file(file&& other) noexcept = default;
    file& operator=(file const& other) noexcept = default;
    file& operator=(file&& other) noexcept = default;

    [[nodiscard]] hi::access_mode access_mode() const noexcept
    {
        return _pimpl->access_mode();
    }

    /** Close the file.
     */
    void close()
    {
        return _pimpl->close();
    }

    /** Flush and block until all data is physically written to disk.
     * Flushing is required before renaming a file, to prevent
     * data corruption when the computer crashes during the rename.
     */
    void flush()
    {
        return _pimpl->flush();
    }

    /** Rename an open file.
     * This function will rename an open file atomically.
     *
     * @param destination The destination file name.
     * @param overwrite_existing Overwrite an existing file.
     * @throw io_error When failing to rename.
     */
    void rename(std::filesystem::path const& destination, bool overwrite_existing = true)
    {
        return _pimpl->rename(destination, overwrite_existing);
    }

    /** Return the size of the file.
     */
    [[nodiscard]] std::size_t size() const
    {
        return _pimpl->size();
    }

    /** Set the seek location.
     * @param offset To move the file pointer.
     * @param whence Where to seek from: begin, current or end
     * @return The new seek position relative to the beginning of the file.
     */
    std::size_t seek(std::ptrdiff_t offset, seek_whence whence = seek_whence::begin)
    {
        return _pimpl->seek(offset, whence);
    }

    /** Get the current seek location.
     */
    std::size_t get_seek()
    {
        return seek(0, seek_whence::current);
    }

    /** Write data to a file.
     *
     * @param data Pointer to data to be written.
     * @param size The number of bytes to write.
     * @throw io_error
     */
    void write(void const *data, std::size_t size)
    {
        return _pimpl->write(data, size);
    }

    /** Read data from a file.
     *
     * @param data Pointer to a buffer to read into.
     * @param size The number of bytes to read.
     * @return The number of bytes read.
     * @throw io_error
     */
    [[nodiscard]] std::size_t read(void *data, std::size_t size)
    {
        return _pimpl->read(data, size);
    }

    /** Write data to a file.
     *
     * @param bytes The byte string to write
     * @throw io_error
     */
    void write(std::span<std::byte const> bytes)
    {
        return write(bytes.data(), ssize(bytes));
    }

    /** Write data to a file.
     *
     * @param text The byte string to write
     * @throw io_error
     */
    void write(bstring_view text)
    {
        return write(text.data(), ssize(text));
    }

    /** Write data to a file.
     *
     * @param text The byte string to write
     * @throw io_error
     */
    void write(bstring const& text)
    {
        return write(text.data(), ssize(text));
    }

    /** Write data to a file.
     *
     * @param text The UTF-8 string to write
     * @throw io_error
     */
    void write(std::string_view text)
    {
        return write(text.data(), ssize(text));
    }

    /** Read bytes from the file.
     *
     * @param max_size The maximum number of bytes to read.
     * @return Data as a byte string, may return less then the requested size.
     * @throws io_error On IO error.
     */
    [[nodiscard]] bstring read_bstring(std::size_t max_size = 10'000'000)
    {
        hilet offset = get_seek();
        hilet size_ = std::min(max_size, this->size() - offset);

        auto r = bstring{};
        // XXX c++23 resize_and_overwrite()
        r.resize(size_);
        hilet bytes_read = read(r.data(), size_);
        r.resize(bytes_read);
        return r;
    }

    /** Read a UTF-8 string from the file.
     *
     * Because of complications with reading UTF-8 string with sequences
     * it is only allowed to read from the start of the file.
     *
     * @note It is undefined bahavior when the seek pointer is not zero.
     * @param max_size The maximum number of bytes to read.
     * @return Data as a UTF-8 string, may return less then the requested size.
     * @throws io_error On IO error.
     */
    [[nodiscard]] std::string read_string(std::size_t max_size = 10'000'000)
    {
        hi_assert(get_seek() == 0);

        hilet size_ = size();
        if (size_ > max_size) {
            throw io_error("read_string() requires the file size to be smaler than max_size.");
        }

        auto r = std::string{};
        // XXX c++23 resize_and_overwrite()
        r.resize(size_);
        hilet bytes_read = read(r.data(), size_);
        r.resize(bytes_read);
        return r;
    }

private:
    std::shared_ptr<detail::file_impl> _pimpl;

    friend class file_view;
};

}} // namespace hi::v1
