// Copyright Jens A. Koch 2021.
// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "win32_headers.hpp"

#include "URL.hpp"
#include "strings.hpp"
#include "utility.hpp"
#include "url_parser.hpp"
#include "log.hpp"
#include "metadata.hpp"
#include <regex>

namespace hi::inline v1 {

/*! Convenience function for SHGetKnownFolderPath().
 *  Retrieves a full path of a known folder identified by the folder's KNOWNFOLDERID.
 *  See https://docs.microsoft.com/en-us/windows/win32/shell/knownfolderid#constants
 *
 * \param KNOWNFOLDERID folder_id.
 * \return The URL of the folder.
 */
static URL get_folder_by_id(const KNOWNFOLDERID &folder_id) noexcept
{
    PWSTR path = nullptr;
    if (SHGetKnownFolderPath(folder_id, 0, nullptr, &path) != S_OK) {
        hi_log_fatal("Could not get known folder path.");
    }
    URL folder = URL::urlFromWPath(path);
    CoTaskMemFree(path);
    return folder;
}

URL URL::urlFromExecutableFile() noexcept
{
    std::wstring module_path;
    auto buffer_size = MAX_PATH; // initial default value = 256
    // iterative buffer resizing to max value of 32768 (256*2^7)
    for (std::size_t i = 0; i < 7; ++i) {
        module_path.resize(buffer_size);
        auto chars = GetModuleFileNameW(nullptr, &module_path[0], buffer_size);
        if (chars < module_path.length()) {
            module_path.resize(chars);
            return URL::urlFromWPath(module_path);
        } else {
            buffer_size *= 2;
        }
    }
    hi_log_fatal("Could not get executable path. It exceeds the buffer length of 32768 chars.");
}

URL URL::urlFromResourceDirectory() noexcept
{
    // Resource path, is the same directory as where the executable lives.
    static auto r = urlFromExecutableDirectory() / "resources";
    return r;
}

URL URL::urlFromApplicationDataDirectory() noexcept
{
    // FOLDERID_LocalAppData has the default path: %LOCALAPPDATA% (%USERPROFILE%\AppData\Local)
    if (metadata::application().vendor.empty()) {
        return get_folder_by_id(FOLDERID_LocalAppData) / metadata::application().display_name;

    } else {
        return get_folder_by_id(FOLDERID_LocalAppData) / metadata::application().vendor / metadata::application().display_name;
    }
}

URL URL::urlFromSystemfontDirectory() noexcept
{
    // FOLDERID_Fonts has the default path: %windir%\Fonts
    return get_folder_by_id(FOLDERID_Fonts);
}

URL URL::urlFromApplicationPreferencesFile() noexcept
{
    return URL::urlFromApplicationDataDirectory() / "preferences.json";
}

std::vector<std::string> URL::filenamesByScanningDirectory(std::string_view path) noexcept
{
    auto searchPath = static_cast<std::string>(path);
    searchPath += '/';
    searchPath += '*';

    std::vector<std::string> filenames;
    WIN32_FIND_DATAW fileData;

    hilet findHandle = FindFirstFileW(URL::nativeWPathFromPath(searchPath).data(), &fileData);
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

} // namespace hi::inline v1
