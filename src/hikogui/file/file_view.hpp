// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file file/file_view.hpp Defines the file_view class.
 * @ingroup file
 */

#pragma once

#include "file_mapping.hpp"
#include "resource_view.hpp"
#include "URI.hpp"
#include "../void_span.hpp"
#include <span>

hi_warning_push();
// C26490: Don't use reinterpret_cast (type.1).
// We need to convert bytes to chars to get a string_view from the byte buffer.
hi_warning_ignore_msvc(26490);

namespace hi { inline namespace v1 {

namespace detail {

class file_view_impl {
public:
    file_view_impl(file_view_impl const &) = delete;
    file_view_impl(file_view_impl &&) = delete;
    file_view_impl&operator=(file_view_impl const &) = delete;
    file_view_impl&operator=(file_view_impl &&) = delete;

    virtual ~file_view_impl() = default;
    file_view_impl(std::shared_ptr<file_impl> file) _file(std::move(file)) {}

    [[nodiscard]] virtual void_span span() noexcept;
    [[nodiscard]] virtual const_void_span const_span() noexcept;

private:
    std::shared_ptr<file_impl> _file;
};

}

/** Map a file into virtual memory.
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
class file_view : public writable_resource_view {
public:
    ~file_view() = default;
    file_view() = delete;
    file_view(file_view const& other) noexcept;
    file_view(file_view&& other) noexcept;
    file_view& operator=(file_view const& other) noexcept;
    file_view& operator=(file_view&& other) noexcept;

    /** Create a file-view from a file-mapping.
     *
     * @note The mapping object will be retained by this file-view.
     * @param mapping A shared pointer to a file-mapping.
     * @param offset The offset from the beginning of the file-mapping (a file mapping may have an offset on its own).
     *               The offset must also be a multiple of the granularity.
     * @param size The size of the mapping, if zero the full file-mapping object is mapped.
     */
    file_view(std::shared_ptr<file_mapping> const& mapping, std::size_t offset = 0, std::size_t size = 0);

    file_view(
        std::filesystem::path const& path,
        access_mode access_mode = access_mode::open_for_read,
        std::size_t offset = 0,
        std::size_t size = 0);

    file_view(
        std::string_view path,
        access_mode access_mode = access_mode::open_for_read,
        std::size_t offset = 0,
        std::size_t size = 0) :
        file_view(std::filesystem::path{path}, access_mode, offset, size)
    {
    }

    file_view(
        std::string const& path,
        access_mode access_mode = access_mode::open_for_read,
        std::size_t offset = 0,
        std::size_t size = 0) :
        file_view(std::filesystem::path{path}, access_mode, offset, size)
    {
    }

    file_view(
        char const *path,
        access_mode access_mode = access_mode::open_for_read,
        std::size_t offset = 0,
        std::size_t size = 0) :
        file_view(std::filesystem::path{path}, access_mode, offset, size)
    {
    }

    /*! Access mode of the opened file.
     */
    [[nodiscard]] access_mode accessMode() const noexcept
    {
        return _file_mapping_object->accessMode();
    }

    /*! URL location to the file.
     */
    [[nodiscard]] std::filesystem::path const& path() const noexcept
    {
        return _file_mapping_object->path();
    }

    /*! Offset of the mapping into the file.
     */
    [[nodiscard]] std::size_t offset() const noexcept override
    {
        return _offset;
    }

    /*! Span to the mapping into memory.
     */
    [[nodiscard]] void_span writable_span() noexcept override
    {
        return *_bytes;
    }

    /*! Span to the mapping into memory.
     */
    [[nodiscard]] const_void_span span() const noexcept override
    {
        return *_bytes;
    }

    /** Flush changes in memory to the open file.
     * \param base Start location of the memory to flush.
     * \param size Number of bytes from the base of the memory region to flush.
     */
    void flush(void *base, std::size_t size);

    /*! Load a view of a resource.
     * This is used when the resource that needs to be opened is a file.
     */
    [[nodiscard]] static std::unique_ptr<resource_view> load_view(std::filesystem::path const& path)
    {
        return std::make_unique<file_view>(path);
    }

private:
    /*! Unmap the bytes from memory.
     * This is used by the shared_ptr to span to automatically unmap the memory, even
     * if the file_mapping has been copied.
     *
     * \param bytes The bytes to unmap.
     */
    static void unmap(void_span *bytes) noexcept;

    /*! Open a file mapping object.
     * File mapping objects are cached and will be shared by file_views.
     * Caching is done using std::weak_ptr to file_mapping objects
     *
     * \param path The path to the file.
     * \param access_mode mode how to open the file.
     * \param size Number of bytes from the start of the file to map.
     * \return A shared-pointer to file mapping object.
     */
    [[nodiscard]] static std::shared_ptr<file_mapping>
    findOrCreateFileMappingObject(std::filesystem::path const& path, access_mode access_mode, std::size_t size);

private:
    /*! pointer to a file mapping object.
     */
    std::shared_ptr<file_mapping> _file_mapping_object;

    /*! A pointer to virtual memory that maps the file into memory.
     * The shared_ptr to _bytes allows the file_view to be copied while pointing
     * to the same memory map. This shared_ptr will use the private unmap().
     */
    std::shared_ptr<void_span> _bytes;

    /*! The offset into the file which is mapped to memory.
     */
    std::size_t _offset;
};

}} // namespace hi::inline v1

hi_warning_pop();
