// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "file.hpp"
#include <memory>
#include <unordered_map>

namespace hi::inline v1 {

/*! A file mapping.
 * A file mapping maps a region of bytes to a handle.
 * This is mostly for Window which has an extra layer of indirection
 * before mapping the file to actual memory.
 * \see file_view on how to map the file into actual memory.
 */
class file_mapping {
public:
    /*! A pointer to an open file.
     */
    std::shared_ptr<file> file;

    /*! Size of the mapping.
     */
    std::size_t size;

    /*! Operating system handle to a file mapping.
     */
    file_handle mapHandle;

    /*! Map a file.
     * Map a file up to size bytes.
     * \param file a pointer to an open file.
     * \param size Number of bytes from the start to map.
     */
    file_mapping(std::shared_ptr<hi::file> const &file, std::size_t size);

    /*! Map a file.
     * Map a file up to size bytes.
     * This function will automatically open a file and potentially
     * share it with other file_mapping objects.
     *
     * \param path a URL to a file.
     * \param accessMode mode of how to access the file.
     * \param size Number of bytes from the start to map.
     */
    file_mapping(URL const &path, access_mode accessMode, std::size_t size);
    ~file_mapping();

    file_mapping(file_mapping const &other) = delete;
    file_mapping(file_mapping &&other) = delete;
    file_mapping &operator=(file_mapping const &other) = delete;
    file_mapping &operator=(file_mapping &&other) = delete;

    /*! Get access mode of the file object.
     */
    [[nodiscard]] access_mode accessMode() const noexcept
    {
        return file->_access_mode;
    }

    /*! Get URL of the file object.
     */
    [[nodiscard]] URL const &location() const noexcept
    {
        return file->_location;
    }

private:
    /*! Find or open a file object.
     * File objects are automatically cached with std:::weak_ptr by this function.
     *
     * \param URL a URL to a file.
     * \param accesssMode mode of how to access the file.
     * \return A shared pointer to a file object.
     */
    [[nodiscard]] static std::shared_ptr<hi::file> findOrOpenFile(URL const &path, access_mode accessMode);
};

} // namespace hi::inline v1
