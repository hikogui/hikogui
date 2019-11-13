// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/url_parser.hpp"
#include <regex>

#include <Windows.h>
#include <ShlObj_core.h>

namespace TTauri {

URL URL::urlFromCurrentWorkingDirectory() noexcept
{
    wchar_t currentDirectory[MAX_PATH];
    if (GetCurrentDirectoryW(MAX_PATH, currentDirectory) == 0) {
        // Can only cause error if there is not enough room in currentDirectory.
        no_default;
    }
    return URL::urlFromWPath(currentDirectory);
}

URL URL::urlFromExecutableFile() noexcept
{
    static auto r = []() {
        wchar_t modulePathWChar[MAX_PATH];
        if (GetModuleFileNameW(nullptr, modulePathWChar, MAX_PATH) == 0) {
            // Can only cause error if there is not enough room in modulePathWChar.
            no_default;
        }
        return URL::urlFromWPath(modulePathWChar);
    }();
    return r;
}

URL URL::urlFromResourceDirectory() noexcept
{
    // Resource path, is the same directory as where the executable lives.
    static auto r = urlFromExecutableDirectory();
    return r;
}

URL URL::urlFromApplicationDataDirectory() noexcept
{
    PWSTR wchar_localAppData;

    // Use application name for the directory inside the application-data directory.
    if (SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &wchar_localAppData) != S_OK) {
        // This should really never happen.
        no_default;
    }

    let base_localAppData = URL::urlFromWPath(wchar_localAppData);
    return base_localAppData / Foundation_globals->applicationName;
}

std::vector<std::string> URL::filenamesByScanningDirectory(std::string_view path) noexcept
{
    auto searchPath = static_cast<std::string>(path);
    searchPath += '/';
    searchPath += '*';

    std::vector<std::string> filenames;
    WIN32_FIND_DATAW fileData;

    let findHandle = FindFirstFileW(URL::nativeWPathFromPath(searchPath).data(), &fileData);
    if (findHandle == INVALID_HANDLE_VALUE) {
        return filenames;
    }

    do {
        auto filename = translateString<std::string>(std::wstring(fileData.cFileName));

        if ((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0) {
            filename += '/';
        } else if ((fileData.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) > 0) {
            continue;
        }

        filenames.push_back(filename);

    } while (FindNextFileW(findHandle, &fileData));

    FindClose(findHandle);
    return filenames;
}

}
