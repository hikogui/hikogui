// Copyright 2019 Pokitec
// All rights reserved.

#include "URL.hpp"
#include "utils.hpp"
#include "strings.hpp"
#include "exceptions.hpp"
#include "os_detect.hpp"
#include "url_parser.hpp"
#include "Application.hpp"
#include <regex>

#if OPERATING_SYSTEM == OS_WINDOWS
#include <Windows.h>
#include <ShlObj_core.h>
#endif

namespace TTauri {

URL URL::urlFromCurrentWorkingDirectory() noexcept
{
#if OPERATING_SYSTEM == OS_WINDOWS
    wchar_t currentDirectory[MAX_PATH];
    if (GetCurrentDirectoryW(MAX_PATH, currentDirectory) == 0) {
        LOG_FATAL("GetCurrentDirectoryW() failed.");
    }
    return URL::urlFromWPath(currentDirectory);
#else
#error "Not implemented urlFromCurrentWorkingDirectory()"
#endif
}

URL URL::urlFromExecutableFile() noexcept
{
#if OPERATING_SYSTEM == OS_WINDOWS
    static auto r = []() {
        wchar_t modulePathWChar[MAX_PATH];
        if (GetModuleFileNameW(nullptr, modulePathWChar, MAX_PATH) == 0) {
            // Can only cause error if there is not enough room in modulePathWChar.
            no_default;
        }
        return URL::urlFromWPath(modulePathWChar);
    }();
    return r;
#else
#error "Not implemented urlFromExecutableFile()"
#endif
}

URL URL::urlFromExecutableDirectory() noexcept
{
    static auto r = urlFromExecutableFile().urlByRemovingFilename();
    return r;
}

URL URL::urlFromResourceDirectory() noexcept
{
    // Resource path, is the same directory as where the executable lives.
    static auto r = urlFromExecutableDirectory();
    return r;
}

std::optional<URL> URL::urlFromApplicationDataDirectory() noexcept
{
    if (let optionalApplicationName = get_singleton<Application>().applicationName()) {
#if OPERATING_SYSTEM == OS_WINDOWS
        PWSTR wchar_localAppData;

        // Use application name for the directory inside the application-data directory.
        if (SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &wchar_localAppData) != S_OK) {
            // This should really never happen.
            no_default;
        }

        let base_localAppData = URL::urlFromWPath(wchar_localAppData);
        return base_localAppData / *optionalApplicationName;
#else
#error "Not Implemented for this operating system."
#endif
    } else {
        return {};
    }
}

std::optional<URL> URL::urlFromApplicationLogDirectory() noexcept
{
    if (let optionalApplicationDataDirectory = urlFromApplicationDataDirectory()) {
        return *optionalApplicationDataDirectory / "Log";
    } else {
        return {};
    }
}

int64_t fileSize(URL const &url)
{
#if OPERATING_SYSTEM == OS_WINDOWS
    let name = url.nativeWPath();

    WIN32_FILE_ATTRIBUTE_DATA attributes;
    if (GetFileAttributesExW(name.data(), GetFileExInfoStandard, &attributes) == 0) {
        TTAURI_THROW(io_error("Could not retrieve file attributes") << error_info<"url"_tag>(url));
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