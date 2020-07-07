// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/File.hpp"
#include <memory>
#include <unordered_map>

namespace tt {

/*! A file mapping.
 * A file mapping maps a region of bytes to a handle.
 * This is mostly for Window which has an extra layer of indirection
 * before mapping the file to actual memory.
 * \see FileView on how to map the file into actual memory.
 */
struct FileMapping {
    /*! A pointer to an open file.
     */
    std::shared_ptr<File> file;

    /*! Size of the mapping.
     */
    size_t size;

    /*! Operating system handle to a file mapping.
     */
    FileHandle mapHandle;

    /*! Map a file.
     * Map a file up to size bytes.
     * \param file a pointer to an open file.
     * \param size Number of bytes from the start to map.
     */
    FileMapping(std::shared_ptr<File> const& file, size_t size);

    /*! Map a file.
     * Map a file up to size bytes.
     * This function will automatically open a file and potentially
     * share it with other FileMapping objects.
     *
     * \param path a URL to a file.
     * \param accessMode mode of how to access the file.
     * \param size Number of bytes from the start to map.
     */
    FileMapping(URL const& path, AccessMode accessMode, size_t size);
    ~FileMapping();

    FileMapping(FileMapping const &other) = delete;
    FileMapping(FileMapping &&other) = delete;
    FileMapping &operator=(FileMapping const &other) = delete;
    FileMapping &operator=(FileMapping &&other) = delete;

    /*! Get access mode of the file object.
     */
    [[nodiscard]] AccessMode accessMode() const noexcept{ return file->accessMode; }

    /*! Get URL of the file object.
     */
    [[nodiscard]] URL const &location() const noexcept { return file->location; }

private:
    /*! Find or open a file object.
     * File objects are automatically cached with std:::weak_ptr by this function.
     *
     * \param URL a URL to a file.
     * \param accesssMode mode of how to access the file.
     * \return A shared pointer to a file object.
     */
    [[nodiscard]] static std::shared_ptr<File> findOrOpenFile(URL const& path, AccessMode accessMode);
};

}
