// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "file_mapping.hpp"
#include "resource_view.hpp"
#include "void_span.hpp"
#include <span>

hi_warning_push();
// C26490: Don't use reinterpret_cast (type.1).
// We need to convert bytes to chars to get a string_view from the byte buffer.
hi_warning_ignore_msvc(26490);

namespace hi::inline v1 {

/*! Map a file into virtual memory.
 */
class file_view : public writable_resource_view {
public:
    file_view(std::shared_ptr<file_mapping> const& mappingObject, std::size_t offset, std::size_t size);
    file_view(
        URL const& location,
        access_mode accessMode = access_mode::open_for_read,
        std::size_t offset = 0,
        std::size_t size = 0);
    ~file_view() = default;

    file_view() = delete;
    file_view(file_view const& other) noexcept;
    file_view(file_view&& other) noexcept;
    file_view& operator=(file_view const& other) noexcept;
    file_view& operator=(file_view&& other) noexcept;

    /*! Access mode of the opened file.
     */
    [[nodiscard]] access_mode accessMode() const noexcept
    {
        return _file_mapping_object->accessMode();
    }

    /*! URL location to the file.
     */
    [[nodiscard]] URL const& location() const noexcept
    {
        return _file_mapping_object->location();
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
    [[nodiscard]] static std::unique_ptr<resource_view> loadView(URL const& location)
    {
        return std::make_unique<file_view>(location);
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
     * \param path URL to the file.
     * \param accessMode mode how to open the file.
     * \param size Number of bytes from the start of the file to map.
     * \return A shared-pointer to file mapping object.
     */
    [[nodiscard]] static std::shared_ptr<file_mapping>
    findOrCreateFileMappingObject(URL const& path, access_mode accessMode, std::size_t size);

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

} // namespace hi::inline v1

hi_warning_pop();
