// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/URL.hpp"
#include <cstdint>
#include <map>

namespace TTauri {


enum class AccessMode {
    Read = 0x1, //!< Allow read access to a file.
    Write = 0x2, //!< Allow write access to a file.
    ReadLock = 0x10, //!< Lock the file for reading, i.e. shared-lock.
    WriteLock = 0x20, //!< Lock the file for writing, i.e. exclusive-lock.
    Open = 0x100, //!< Open file if it exist, or fail.
    Create = 0x200, //!< Create file if it does not exist, or fail.
    Truncate = 0x400, //!< After the file has been opened, truncate it.
    Random = 0x1000, //!< Hint the data should not be prefetched.
    Sequential = 0x2000, //!< Hint that the data should be prefetched.
    NoReuse = 0x4000, //!< Hint that the data should not be cached.
    WriteThrough = 0x8000, //!< Hint that writes should be send directly to disk.
    
    OpenForRead = 0x101, //!< Default open a file for reading.
    OpenForReadWrite = 0x103, //!< Default open a file for reading and writing.
};

[[nodiscard]] inline AccessMode operator|(AccessMode lhs, AccessMode rhs) noexcept
{
    return static_cast<AccessMode>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

[[nodiscard]] inline AccessMode operator&(AccessMode lhs, AccessMode rhs) noexcept
{
    return static_cast<AccessMode>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

/*! True if all bits on rhs are set in lhs.
 */
[[nodiscard]] inline bool operator>=(AccessMode lhs, AccessMode rhs) noexcept
{
    return (lhs & rhs) == rhs;
}

/*! A File object.
 */
struct File {
    /*! The access mode used to open the file.
     */
    AccessMode accessMode;

    /*! The URL that was used to open the file.
     */
    URL location;

    /*! A operating system handle to the file.
     */
    FileHandle fileHandle;

    /*! Open a file at location.
     * \param location The file: URL locating the file.
     * \param accessMode access-mode to open the file.
     */
    File(URL const& location, AccessMode accessMode);

    ~File() noexcept;

    File(File const &other) = delete;
    File(File &&other) = delete;
    File &operator=(File const &other) = delete;
    File &operator=(File &&other) = delete;

    /*! Close the file.
     */
    void close();

    /*! Get the size of a file on the file system.
     * \return The size of the file in bytes.
     */
    [[nodiscard]] static size_t fileSize(URL const &url);
};


}
