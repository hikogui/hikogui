// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "URL.hpp"
#include "strings.hpp"
#include "required.hpp"
#include "url_parser.hpp"
#include "ttauri/current_version.hpp"
#include <regex>

#include <Windows.h>
#include <ShlObj_core.h>

namespace tt {

URL URL::urlFromCurrentWorkingDirectory() noexcept
{
    wchar_t currentDirectory[MAX_PATH];
    if (GetCurrentDirectoryW(MAX_PATH, currentDirectory) == 0) {
        // Can only cause error if there is not enough room in currentDirectory.
        tt_no_default();
    }
    return URL::urlFromWPath(currentDirectory);
}

URL URL::urlFromExecutableFile() noexcept
{
    static auto r = []() {
        wchar_t modulePathWChar[MAX_PATH];
        if (GetModuleFileNameW(nullptr, modulePathWChar, MAX_PATH) == 0) {
            // Can only cause error if there is not enough room in modulePathWChar.
            tt_no_default();
        }
        return URL::urlFromWPath(modulePathWChar);
    }();
    return r;
}

URL URL::urlFromResourceDirectory() noexcept
{
    // Resource path, is the same directory as where the executable lives.
    static auto r = urlFromExecutableDirectory() / "resources";
    return r;
}

URL URL::urlFromApplicationDataDirectory() noexcept
{
    PWSTR wchar_localAppData;

    // Use application name for the directory inside the application-data directory.
    if (SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &wchar_localAppData) != S_OK) {
        // This should really never happen.
        tt_no_default();
    }

    ttlet base_localAppData = URL::urlFromWPath(wchar_localAppData);
    return base_localAppData / application_version.name;
}

URL URL::urlFromSystemfontDirectory() noexcept
{
    PWSTR wchar_fonts;

    // Use application name for the directory inside the application-data directory.
    if (SHGetKnownFolderPath(FOLDERID_Fonts, 0, nullptr, &wchar_fonts) != S_OK) {
        // This should really never happen.
        tt_no_default();
    }

    return URL::urlFromWPath(wchar_fonts);
}

std::vector<std::string> URL::filenamesByScanningDirectory(std::string_view path) noexcept
{
    auto searchPath = static_cast<std::string>(path);
    searchPath += '/';
    searchPath += '*';

    std::vector<std::string> filenames;
    WIN32_FIND_DATAW fileData;

    ttlet findHandle = FindFirstFileW(URL::nativeWPathFromPath(searchPath).data(), &fileData);
    if (findHandle == INVALID_HANDLE_VALUE) {
        return filenames;
    }

    do {
        auto filename = to_string(std::wstring(fileData.cFileName));

        if (filename == "." || filename == "..") {
            continue;
        } else if ((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0) {
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
