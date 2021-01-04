// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "FileMapping.hpp"
#include "ResourceView.hpp"
#include <span>

namespace tt {

/*! Map a file into virtual memory.
 */
class FileView : public ResourceView {
private:
    /*! pointer to a file mapping object.
     */
    std::shared_ptr<FileMapping> fileMappingObject;

    /*! A pointer to virtual memory that maps the file into memory.
     * The shared_ptr to _bytes allows the FileView to be copied while pointing
     * to the same memory map. This shared_ptr will use the private unmap().
     */
    std::shared_ptr<std::span<std::byte>> _bytes;

    /*! The offset into the file which is mapped to memory.
     */
    size_t _offset;

public:
    FileView(std::shared_ptr<FileMapping> const& mappingObject, size_t offset, size_t size);
    FileView(URL const &location, access_mode accessMode=access_mode::open_for_read, size_t offset=0, size_t size=0);
    ~FileView() = default;

    FileView() = delete;
    FileView(FileView const &other) noexcept;
    FileView(FileView &&other) noexcept;
    FileView &operator=(FileView const &other) noexcept;
    FileView &operator=(FileView &&other) noexcept;

    /*! Access mode of the opened file.
     */
    [[nodiscard]] access_mode accessMode() const noexcept { return fileMappingObject->accessMode(); }

    /*! URL location to the file.
     */
    [[nodiscard]] URL const &location() const noexcept { return fileMappingObject->location(); }

    /*! Offset of the mapping into the file.
     */
    [[nodiscard]] size_t offset() const noexcept override { return _offset; }

    /*! Number of bytes which is mapped to memory.
     */
    [[nodiscard]] size_t size() const noexcept override { return _bytes->size(); }

    /*! Pointer to the mapping into memory.
     */
    [[nodiscard]] std::byte *data() noexcept { return _bytes->data(); }

    /*! Pointer to the mapping into memory.
     */
    [[nodiscard]] std::byte const *data() const noexcept override { return _bytes->data(); }

    /*! Span to the mapping into memory.
     */
    [[nodiscard]] std::span<std::byte> bytes() noexcept { return *_bytes; }

    /*! Span to the mapping into memory.
     */
    [[nodiscard]] std::span<std::byte const> bytes() const noexcept override { return *_bytes; }

    /*! String view to the mapping into memory.
     */
    [[nodiscard]] std::string_view string_view() noexcept {
        return std::string_view{reinterpret_cast<char *>(data()), size()};
    }

    /*! String view to the mapping into memory.
     */
    [[nodiscard]] std::string_view string_view() const noexcept override {
        return std::string_view{reinterpret_cast<char const *>(data()), size()};
    }

    /** Flush changes in memory to the open file.
     * \param base Start location of the memory to flush.
     * \param size Number of bytes from the base of the memory region to flush.
     */
    void flush(void* base, size_t size);

    /*! Load a view of a resource.
     * This is used when the resource that needs to be opened is a file.
     */
    [[nodiscard]] static std::unique_ptr<ResourceView> loadView(URL const &location) {
        return std::make_unique<FileView>(location);
    }

private:
    /*! Unmap the bytes from memory.
     * This is used by the shared_ptr to span to automatically unmap the memory, even
     * if the FileMapping has been copied.
     *
     * \param bytes The bytes to unmap.
     */
    static void unmap(std::span<std::byte> *bytes) noexcept;

    /*! Open a file mapping object.
     * File mapping objects are cached and will be shared by FileViews.
     * Caching is done using std::weak_ptr to FileMapping objects
     *
     * \param path URL to the file.
     * \param accessMode mode how to open the file.
     * \param size Number of bytes from the start of the file to map.
     * \return A shared-pointer to file mapping object.
     */
    [[nodiscard]] static std::shared_ptr<FileMapping> findOrCreateFileMappingObject(URL const& path, access_mode accessMode, size_t size);
};

}
