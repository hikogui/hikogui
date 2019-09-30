// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/File.hpp"
#include "TTauri/Diagnostic/logger.hpp"
#include "TTauri/Diagnostic/exceptions.hpp"
#include "TTauri/Required/strings.hpp"
#include <Windows.h>

namespace TTauri {

File::File(URL const &location, AccessMode accessMode) :
    accessMode(accessMode), location(location)
{
    DWORD desiredAccess = 0;
    if (accessMode >= (AccessMode::Read | AccessMode::Write)) {
        desiredAccess = GENERIC_READ | GENERIC_WRITE;
    } else if (accessMode >= AccessMode::Read) {
        desiredAccess = GENERIC_READ;
    } else if (accessMode >= AccessMode::Write) {
        desiredAccess = GENERIC_WRITE;
    } else {
        TTAURI_THROW(io_error("Invalid AccessMode; expecting Readable and/or Writeable.")
            .set<"url"_tag>(location)
        );
    }

    DWORD shareMode;
    if (accessMode >= AccessMode::WriteLock) {
        shareMode = 0;
    } else if (accessMode >= AccessMode::ReadLock) {
        shareMode = FILE_SHARE_READ;
    } else {
        // Allow files to be renamed and deleted.
        shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    }

    DWORD creationDisposition;
    if (accessMode >= (AccessMode::Create | AccessMode::Open)) {
        if (accessMode >= AccessMode::Truncate) {
            creationDisposition = CREATE_ALWAYS;
        } else {
            creationDisposition = OPEN_ALWAYS;
        }

    } else if (accessMode >= AccessMode::Create) {
        creationDisposition = CREATE_NEW;

    } else if (accessMode >= AccessMode::Open) {
        if (accessMode >= AccessMode::Truncate) {
            creationDisposition = TRUNCATE_EXISTING;
        } else {
            creationDisposition = OPEN_EXISTING;
        }

    } else {
        TTAURI_THROW(io_error("Invalid AccessMode; expecting CreateFile and/or OpenFile.")
            .set<"url"_tag>(location)
        );
    }

    DWORD flagsAndAttributes = 0;
    if (accessMode >= AccessMode::Random) {
        flagsAndAttributes |= FILE_FLAG_RANDOM_ACCESS;
    }
    if (accessMode >= AccessMode::Sequential) {
        flagsAndAttributes |= FILE_FLAG_SEQUENTIAL_SCAN;
    }
    if (accessMode >= AccessMode::WriteThrough) {
        flagsAndAttributes |= FILE_FLAG_WRITE_THROUGH;
    }

    let fileName = location.nativeWPath();
    if ((fileHandle = CreateFileW(fileName.data(), desiredAccess, shareMode, NULL, creationDisposition, flagsAndAttributes, NULL)) == INVALID_HANDLE_VALUE) {
        TTAURI_THROW(io_error("Could not open file")
            .set<"error_message"_tag>(getLastErrorMessage())
            .set<"url"_tag>(location)
        );
    }
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
    let name = url.nativeWPath();

    WIN32_FILE_ATTRIBUTE_DATA attributes;
    if (GetFileAttributesExW(name.data(), GetFileExInfoStandard, &attributes) == 0) {
        TTAURI_THROW(io_error("Could not retrieve file attributes").set<"url"_tag>(url));
    }

    LARGE_INTEGER size;
    size.HighPart = attributes.nFileSizeHigh;
    size.LowPart = attributes.nFileSizeLow;
    return numeric_cast<int64_t>(size.QuadPart);
}

}
