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
            return URL("none:");
        default:
            no_default;
        }
    }
    return URL::urlFromPath(currentDirectory);
}

URL URL::urlFromExecutableFile() noexcept
{
    uint32_t executablePathLength = MAXPATHLEN;

    char *executablePath = new char[executablePathLength];

    if (_NSGetExecutablePath(executablePath, &executablePathLength) == -1) {
        // Not enough room, try again.
        delete [] executablePath;
        executablePath = new char[executablePathLength];

        if (_NSGetExecutablePath(executablePath, &executablePathLength) == -1) {
            no_default;
        }
    }

    auto url = URL::urlFromPath(executablePath);

    delete [] executablePath;
    return url;
}

URL URL::urlFromResourceDirectory() noexcept
{
    NSURL *url = [NSBundle.mainBundle resourceURL];
    return URL::urlFromPath(url.absoluteString.UTF8String);
}

URL URL::urlFromApplicationDataDirectory() noexcept
{
    NSArray *urls = [NSFileManager.defaultManager URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];
    NSURL *url = urls.firstObject;
    return URL::urlFromPath(url.absoluteString.UTF8String);
}

}
