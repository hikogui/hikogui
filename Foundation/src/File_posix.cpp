// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/File.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/strings.hpp"
#include <fcntl.h>

namespace TTauri {

File::File(URL const &location, AccessMode accessMode) :
    accessMode(accessMode), location(location)
{
    int openFlags = 0;

    if (accessMode >= (AccessMode::Read | AccessMode::Write)) {
        openFlags = O_RDWR;
    } else if (accessMode >= AccessMode::Read) {
        openFlags = O_RDONLY;;
    } else if (accessMode >= AccessMode::Write) {
        openFlags = O_WRONLY;;
    } else {
        TTAURI_THROW(io_error("Invalid AccessMode; expecting Readable and/or Writeable.")
            .set<"url"_tag>(location)
        );
    }

    if (accessMode >= AccessMode::WriteLock) {
        openFlags |= O_EXLOCK;
    } else if (accessMode >= AccessMode::ReadLock) {
        openFlags |= O_SHLOCK;
    }

    if (accessMode >= (AccessMode::Create | AccessMode::Open)) {
        openFlags |= O_CREAT;
        if (accessMode >= AccessMode::Truncate) {
            openFlags |= O_TRUNC;
        }

    } else if (accessMode >= AccessMode::Create) {
        openFlags |= (O_CREAT | O_EXCL);

    } else if (accessMode >= AccessMode::Open) {
        if (accessMode >= AccessMode::Truncate) {
            openFlags |= O_TRUNC;
        }

    } else {
        TTAURI_THROW(io_error("Invalid AccessMode; expecting CreateFile and/or OpenFile.")
            .set<"url"_tag>(location)
        );
    }

    int advise = 0;
    if (accessMode >= AccessMode::Random) {
        advise = POSIX_FADV_RANDOM;
    } if (accessMode >= AccessMode::Sequential) {
        advise = POSIX_FADV_SEQUENTIAL;
    } else {
        advise = 0;
    }

    if (accessMode >= AccessMode::NoReuse) {
        advise |= POSIX_FADV_NOREUSE;
    }

    int permissions = 0;

    let fileName = location.nativePath();
    if ((fileHandle = open(fileName.data(), openFlag, permissions) == -1) {
        TTAURI_THROW(io_error("Could not open file")
            .set<"error_message"_tag>(getLastErrorMessage())
            .set<"url"_tag>(location)
        );
    }
#endif
}

File::~File() noexcept
{
    close();
}

void File::close()
{
    if (fileHandle != INVALID_HANDLE_VALUE) {
        if (!CloseHandle(fileHandle)) {
            TTAURI_THROW(io_error("Could not close file")
                .set<"error_message"_tag>(getLastErrorMessage())
                .set<"url"_tag>(location)
            );
        }
        fileHandle = INVALID_HANDLE_VALUE;
    }
}

int64_t File::fileSize(URL const &url)
{
#if OPERATING_SYSTEM == OS_WINDOWS
    let name = url.nativeWPath();

    WIN32_FILE_ATTRIBUTE_DATA attributes;
    if (GetFileAttributesExW(name.data(), GetFileExInfoStandard, &attributes) == 0) {
        TTAURI_THROW(io_error("Could not retrieve file attributes").set<"url"_tag>(url));
    }

    LARGE_INTEGER size;
    size.HighPart = attributes.nFileSizeHigh;
    size.LowPart = attributes.nFileSizeLow;
    return numeric_cast<int64_t>(size.QuadPart);
#else
#error "Not Implemented for this operating system."
#endif
}

}
