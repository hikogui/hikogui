// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/URL.hpp"


#include <cstdint>
#include <map>

namespace TTauri {


enum class AccessMode {
    Read = 0x1,
    Write = 0x2,
    ReadLock = 0x10,
    WriteLock = 0x20,
    Open = 0x100, //!< Open file if it exist, or fail.
    Create = 0x200, //!< Create file if it does not exist, or fail.
    Truncate = 0x400, //!< After the file has been opened, truncate it.
    Random = 0x1000, //!< Hint the operating system that the data in the file will be accessed with random read or write patterns.
    Sequential = 0x2000, //!< Hint to the operating system that the data in the file will be accessed sequentially.
    WriteThrough = 0x4000, //!< Hint to the operating system that writes should be commited to disk quickly.

    OpenForRead = 0x101,
    OpenForReadWrite = 0x103,
};

inline AccessMode operator|(AccessMode lhs, AccessMode rhs) noexcept
{
    return static_cast<AccessMode>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline AccessMode operator&(AccessMode lhs, AccessMode rhs) noexcept
{
    return static_cast<AccessMode>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

inline bool operator==(AccessMode lhs, AccessMode rhs) noexcept
{
    return static_cast<int>(lhs) == static_cast<int>(rhs);
}

inline bool operator<(AccessMode lhs, AccessMode rhs) noexcept
{
    return (lhs & rhs) != rhs;
}

inline bool operator!=(AccessMode lhs, AccessMode rhs) noexcept { return !(lhs == rhs); }
inline bool operator>=(AccessMode lhs, AccessMode rhs) noexcept { return !(lhs < rhs); }

struct File {
    AccessMode accessMode;
    URL location;

    void *fileHandle;

    File(URL const& location, AccessMode accessMode);
    ~File() noexcept;

    File(File const &other) = delete;
    File(File &&other) = delete;
    File &operator=(File const &other) = delete;
    File &operator=(File &&other) = delete;

    void close();

    /*! Get the size of a file on the file system.
    */
    static int64_t fileSize(URL const &url);
};


}
