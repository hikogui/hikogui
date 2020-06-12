// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/url_parser.hpp"
#include <regex>

#import <Foundation/Foundation.h>
#include <unistd.h>
#include <mach-o/dyld.h>

namespace tt {

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
    @autoreleasepool {
        NSURL *url = [NSBundle.mainBundle resourceURL];
        return URL::urlFromPath(url.absoluteString.UTF8String);
    }
}

URL URL::urlFromApplicationDataDirectory() noexcept
{
    @autoreleasepool {
        NSArray *urls = [NSFileManager.defaultManager
            URLsForDirectory:NSApplicationSupportDirectory
            inDomains:NSUserDomainMask
        ];
        NSURL *url = urls.firstObject;
        return URL::urlFromPath(url.absoluteString.UTF8String);
    }
}

}
