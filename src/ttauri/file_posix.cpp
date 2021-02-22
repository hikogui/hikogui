// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "file.hpp"
#include "logger.hpp"
#include "exception.hpp"
#include "strings.hpp"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace tt {

File::File(URL const &location, AccessMode accessMode) :
    accessMode(accessMode), location(location)
{
    int openFlags = 0;

    if (accessMode >= (AccessMode::Read | AccessMode::Write)) {
        openFlags = O_RDWR;
    } else if (accessMode >= AccessMode::Read) {
        openFlags = O_RDONLY;;
    } else if (accessMode >= AccessMode::Write) {
        openFlags = O_WRONLY;;
    } else {
        throw io_error("{}: Invalid AccessMode; expecting Readable and/or Writeable.", location());
    }

    if (accessMode >= AccessMode::WriteLock) {
        openFlags |= O_EXLOCK;
    } else if (accessMode >= AccessMode::ReadLock) {
        openFlags |= O_SHLOCK;
    }

    if (accessMode >= (AccessMode::Create | AccessMode::Open)) {
        openFlags |= O_CREAT;
        if (accessMode >= AccessMode::Truncate) {
            openFlags |= O_TRUNC;
        }

    } else if (accessMode >= AccessMode::Create) {
        openFlags |= (O_CREAT | O_EXCL);

    } else if (accessMode >= AccessMode::Open) {
        if (accessMode >= AccessMode::Truncate) {
            openFlags |= O_TRUNC;
        }

    } else {
        throw io_error("{}: Invalid AccessMode; expecting CreateFile and/or OpenFile.", location());
    }

    //int advise = 0;
    //if (accessMode >= AccessMode::Random) {
    //    advise = POSIX_FADV_RANDOM;
    //} if (accessMode >= AccessMode::Sequential) {
    //    advise = POSIX_FADV_SEQUENTIAL;
    //} else {
    //    advise = 0;
    //}
    //
    //if (accessMode >= AccessMode::NoReuse) {
    //    advise |= POSIX_FADV_NOREUSE;
    //}

    int permissions = 0666;

    ttlet fileName = location.nativePath();
    if ((fileHandle = ::open(fileName.data(), openFlags, permissions)) == -1) {
        throw io_error("{}: Could not open file. '{}'", location(), get_last_error_message());
    }
}

File::~File() noexcept
{
    close();
}

void File::close()
{
    if (fileHandle != -1) {
        if (::close(fileHandle) != 0) {
            throw io_error("{}: Could not close file. '{}'", location(), get_last_error_message());
        }
        fileHandle = -1;
    }
}

size_t File::fileSize(URL const &url)
{
    ttlet name = url.nativePath();

    struct ::stat statbuf;

    if (::stat(name.data(), &statbuf) == -1) {
        throw io_error("{}: Could not retrieve file attributes. '{}'", location(), get_last_error_message());
    }

    return narrow_cast<size_t>(statbuf.st_size);
}

}
