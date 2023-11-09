
#pragma once

#include "../macros.hpp"
#include "../utility/utility.hpp"
#include <utility>

hi_export_module(hikogui.file.access_mode);

hi_export namespace hi { inline namespace v1 {

/** The mode in which way to open a file.
 * @ingroup file
 *
 * These flags can be combined by using OR.
 */
hi_export enum class access_mode {
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

hi_export [[nodiscard]] constexpr access_mode operator|(access_mode const& lhs, access_mode const& rhs) noexcept
{
    return static_cast<access_mode>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

hi_export [[nodiscard]] constexpr access_mode operator&(access_mode const& lhs, access_mode const& rhs) noexcept
{
    return static_cast<access_mode>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

hi_export [[nodiscard]] constexpr bool to_bool(access_mode const& rhs) noexcept
{
    return to_bool(std::to_underlying(rhs));
}

}}
