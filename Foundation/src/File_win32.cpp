// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/File.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/strings.hpp"
#include <Windows.h>

namespace tt {

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
            .set<url_tag>(location)
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
            .set<url_tag>(location)
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

    ttlet fileName = location.nativeWPath();
    if ((fileHandle = CreateFileW(fileName.data(), desiredAccess, shareMode, NULL, creationDisposition, flagsAndAttributes, NULL)) != INVALID_HANDLE_VALUE) {
        return;
    }

    ttlet error = GetLastError();
    if (accessMode >= AccessMode::CreateDirectories && error == ERROR_PATH_NOT_FOUND && (
        creationDisposition == CREATE_ALWAYS || creationDisposition == OPEN_ALWAYS || creationDisposition == CREATE_NEW
    )) {
        // Retry opening the file, by first creating the directory hierarchy.
        ttlet directory = location.urlByRemovingFilename();
        File::createDirectoryHierarchy(directory);

        if ((fileHandle = CreateFileW(fileName.data(), desiredAccess, shareMode, NULL, creationDisposition, flagsAndAttributes, NULL)) != INVALID_HANDLE_VALUE) {
            return;
        }
    }

    TTAURI_THROW(io_error("Could not open file")
        .set<error_message_tag>(getLastErrorMessage())
        .set<url_tag>(location)
    );
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
                .set<error_message_tag>(getLastErrorMessage())
                .set<url_tag>(location)
            );
        }
        fileHandle = INVALID_HANDLE_VALUE;
    }
}

/*! Write data to a file.
*/
ssize_t File::write(std::byte const *data, ssize_t size)
{
    ttauri_assume(size >= 0);
    ttauri_assume(fileHandle != INVALID_HANDLE_VALUE);

    ssize_t total_written_size = 0;
    while (size) {
        ttlet write_size = static_cast<DWORD>(std::min(size, static_cast<ssize_t>(std::numeric_limits<DWORD>::max())));
        DWORD written_size = 0;

        if (!WriteFile(fileHandle, data, write_size, &written_size, nullptr)) {
            TTAURI_THROW(io_error("Could not write to file")
                .set<error_message_tag>(getLastErrorMessage())
                .set<url_tag>(location)
            );
        } else if (written_size == 0) {
            break;
        }

        data += written_size;
        size -= written_size;
        total_written_size += written_size;
    }

    return total_written_size;
}


size_t File::fileSize(URL const &url)
{
    ttlet name = url.nativeWPath();

    WIN32_FILE_ATTRIBUTE_DATA attributes;
    if (GetFileAttributesExW(name.data(), GetFileExInfoStandard, &attributes) == 0) {
        TTAURI_THROW(io_error("Could not retrieve file attributes").set<url_tag>(url));
    }

    LARGE_INTEGER size;
    size.HighPart = attributes.nFileSizeHigh;
    size.LowPart = attributes.nFileSizeLow;
    return numeric_cast<int64_t>(size.QuadPart);
}

void File::createDirectory(URL const &url, bool hierarchy)
{
    if (url.isRootDirectory()) {
        TTAURI_THROW(io_error("Cannot create a root directory."));
    }

    ttlet directory_name = url.nativeWPath();
    if (CreateDirectoryW(directory_name.data(), nullptr)) {
        return;
    }

    if (hierarchy && GetLastError() == ERROR_PATH_NOT_FOUND) {
        try {
            File::createDirectory(url.urlByRemovingFilename(), true);
        } catch (io_error &e) {
            e.set<url_tag>(url);
            throw;
        }

        if (CreateDirectoryW(directory_name.data(), nullptr)) {
            return;
        }
    }

    TTAURI_THROW(io_error("Could not create directory")
        .set<error_message_tag>(getLastErrorMessage())
        .set<url_tag>(url)
    );
}

void File::createDirectoryHierarchy(URL const &url)
{
    return createDirectory(url, true);
}

}
