// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Required/globals.hpp"
#include "TTauri/Required/URL.hpp"
#include "TTauri/Required/strings.hpp"
#include "TTauri/Required/required.hpp"
#include "TTauri/Required/url_parser.hpp"
#include <regex>

#import <Foundation/Foundation.h>
#include <unistd.h>
#include <mach-o/dyld.h>

namespace TTauri {

URL URL::urlFromCurrentWorkingDirectory() noexcept
{
    char currentDirectory[MAXPATHLEN];
    if (getcwd(currentDirectory, MAXPATHLEN) == NULL) {
        switch (errno) {
        case ENOENT:
            // Directory no longer exists.
            return URL::urlFromPath("/tmp");
        default:
            no_default;
        }
    }
    return URL::urlFromPath(currentDirectory);
}

URL URL::urlFromExecutableFile() noexcept
{
    char executablePath[MAXPATHLEN];
    uint32_t executablePathLength = MAXPATHLEN;
    if (_NSGetExecutablePath(executablePath, &executablePathLength) == -1) {
        // Can only cause error if there is not enough room in executablePath
        // deep paths could excede MAXPATHLEN.
        no_default;
    }
    return URL::urlFromPath(executablePath);
}

URL URL::urlFromResourceDirectory() noexcept
{
    // Resource path, is the same directory as where the executable lives.
    static auto r = urlFromExecutableDirectory();
    return r;
}

URL URL::urlFromApplicationDataDirectory() noexcept
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *applicationSupportDirectory = [paths firstObject];
    char const *applicationSupportDirectory_cstr = [applicationSupportDirectory UTF8String];

    return URL::urlFromPath(applicationSupportDirectory_cstr);
}

}
