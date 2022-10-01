// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file file/file_mapping.hpp Defines the file_mapping class.
 * @ingroup file
 */

#pragma once

#include "file.hpp"
#include <memory>
#include <unordered_map>

namespace hi { inline inline v1 {

namespace detail {

struct file_mapping_impl {
    std::shared_ptr<file_impl> file;

    virtual ~file_mapping_impl() = default;
    file_mapping_impl(file_mapping_impl const &) = delete;
    file_mapping_impl(file_mapping_impl &&) = delete;
    file_mapping_impl &operator=(file_mapping_impl const &) = delete;
    file_mapping_impl &operator=(file_mapping_impl &&) = delete;

    file_mapping_impl(std::shared_ptr<file_impl> file) : file(std::move(file)) {}

    [[nodiscard]] hi::access_mode access_mode() const noexcept
    {
        return file->access_mode;
    }

    [[nodiscard]] std::filesystem::path const &path() const noexcept
    {
        return file->path;
    }

}

/** A file mapping.
 *
 * A file mapping maps a region of bytes to a handle. It is an
 * intermediate between a `file` and a `file_view`.
 *
 * In most cases you do not need to handle `file_mapping` in your
 * application, as `file_view` will automatically create the `file_mapping`
 * automatically.
 *
 * @ingroup file
 * @see file_view on how to map the file into actual memory.
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
     * \param path The path to the file to map.
     * \param access_mode mode of how to access the file.
     * \param size Number of bytes from the start to map.
     */
    file_mapping(std::filesystem::path const &path, access_mode access_mode, std::size_t size);
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

    /** Get the path to the file object.
     */
    [[nodiscard]] std::filesystem::path const &path() const noexcept
    {
        return file->_path;
    }

private:
    /*! Find or open a file object.
     * File objects are automatically cached with std:::weak_ptr by this function.
     *
     * \param path The path to the file to open.
     * \param accesssMode mode of how to access the file.
     * \return A shared pointer to a file object.
     */
    [[nodiscard]] static std::shared_ptr<hi::file> findOrOpenFile(std::filesystem::path const &path, access_mode accessMode);
};

}} // namespace hi::inline v1

