// Copyright 2019 Pokitec
// All rights reserved.

#include "File.hpp"
#include "strings.hpp"
#include "utils.hpp"
#include "logging.hpp"

namespace TTauri {



File::File(const std::filesystem::path &path, AccessMode accessMode) :
    path(path), accessMode(accessMode)
{
#ifdef WIN32
    auto fileName = path.wstring();

    DWORD desiredAccess = 0;
    if (accessMode >= AccessMode::RDONLY) {
        desiredAccess |= GENERIC_READ;
    }
    if (accessMode >= AccessMode::WRONLY) {
        desiredAccess |= GENERIC_WRITE;
    }

    DWORD shareMode;
    if (accessMode >= AccessMode::WRLOCK) {
        shareMode = 0;
    } else if (accessMode >= AccessMode::WRLOCK) {
        shareMode = FILE_SHARE_READ;
    } else {
        shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    }

    DWORD creationDisposition;
    if (accessMode >= AccessMode::EXCL) {
        creationDisposition = CREATE_NEW;
    } else if (accessMode >= (AccessMode::CREAT | AccessMode::TRUNC)) {
        creationDisposition = CREATE_ALWAYS;
    } else if (accessMode >= AccessMode::CREAT) {
        creationDisposition = OPEN_ALWAYS;
    } else if (accessMode >= AccessMode::TRUNC) {
        creationDisposition = TRUNCATE_EXISTING;
    } else {
        creationDisposition = OPEN_EXISTING;
    }

    DWORD flagsAndAttributes = 0;
    if (accessMode >= AccessMode::RANDOM_ACCESS) {
        flagsAndAttributes |= FILE_FLAG_RANDOM_ACCESS;
    }
    if (accessMode >= AccessMode::SEQUENTIAL) {
        flagsAndAttributes |= FILE_FLAG_SEQUENTIAL_SCAN;
    }
    if (accessMode >= AccessMode::WRITE_THROUGH) {
        flagsAndAttributes |= FILE_FLAG_WRITE_THROUGH;
    }

    if ((intrinsic = CreateFileW(fileName.data(), desiredAccess, shareMode, NULL, creationDisposition, flagsAndAttributes, NULL)) == INVALID_HANDLE_VALUE) {
        BOOST_THROW_EXCEPTION(FileError(getLastErrorMessage()) <<
            boost::errinfo_file_name(path.string())
        );
    }
#endif
}

File::~File()
{
    if (!CloseHandle(intrinsic)) {
        LOG_ERROR("Could not close file '%s'") % getLastErrorMessage();
    }
}

}
