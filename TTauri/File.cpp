// Copyright 2019 Pokitec
// All rights reserved.

#include "File.hpp"
#include "strings.hpp"
#include "utils.hpp"
#include "Logging.hpp"

namespace TTauri {

std::map<std::filesystem::path, std::vector<std::weak_ptr<File>>> FileMapping::mappedFiles;
std::map<std::filesystem::path, std::vector<std::weak_ptr<FileMapping>>> FileView::mappedFileObjects;

static std::string getLastErrorMessage()
{
    DWORD const errorCode = GetLastError();
    size_t const messageSize = 32768;
    wchar_t *const c16_message = new wchar_t[messageSize];;

    FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, // source
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        c16_message,
        messageSize,
        NULL
    );

    auto const message = translateString<std::string>(std::wstring(c16_message));
    delete c16_message;

    return message;
}

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


FileMapping::FileMapping(std::shared_ptr<File> const &file, size_t size) :
    file(file), size(size > 0 ? size : std::filesystem::file_size(file->path))
{
#ifdef WIN32
    DWORD protect;
    if (accessMode() >= AccessMode::RDWR) {
        protect = PAGE_READWRITE;
    } else if (accessMode() >= AccessMode::RDONLY) {
        protect = PAGE_READONLY;
    } else {
        BOOST_THROW_EXCEPTION(FileError("Illigal access mode WRONLY/0 when mapping file.") <<
            boost::errinfo_file_name(path().string())
        );
    }

    DWORD maximumSizeHigh = this->size >> 32;
    DWORD maximumSizeLow = this->size & 0xffffffff;

    if ((intrinsic = CreateFileMappingA(file->intrinsic, NULL, protect, maximumSizeHigh, maximumSizeLow, NULL)) == NULL) {
        BOOST_THROW_EXCEPTION(FileError(getLastErrorMessage()) <<
            boost::errinfo_file_name(path().string())
        );
    }
#endif
}

FileMapping::FileMapping(const std::filesystem::path &path, AccessMode accessMode, size_t size) :
    FileMapping(findOrCreateFile(path, accessMode), size) {}

FileMapping::~FileMapping()
{
    if (!CloseHandle(intrinsic)) {
        LOG_ERROR("Could not close file mapping object on file '%s'") % getLastErrorMessage();
    }
}

std::shared_ptr<File> FileMapping::findOrCreateFile(std::filesystem::path const &path, AccessMode accessMode)
{
    cleanup();

    auto const absolutePath = std::filesystem::absolute(path);

    auto& files = mappedFiles[absolutePath];
    for (auto const weak_file : files) {
        if (auto file = weak_file.lock()) {
            if (file->accessMode >= accessMode) {
                return file;
            }
        }
    }

    auto file = make_shared<File>(path, accessMode);
    files.push_back(file);
    return file;
}

void FileMapping::cleanup()
{
    for (auto &[key, files] : mappedFiles) {
        erase_if(files, [](auto x) {
            return x.expired();
        });
    }

    erase_if(mappedFiles, [](auto x) {
        return x.second.size() == 0;
    });
}

FileView::FileView(std::shared_ptr<FileMapping> const &fileMappingObject, size_t offset, size_t size) :
    fileMappingObject(fileMappingObject),
    offset(offset),
    size(size > 0 ? size : std::filesystem::file_size(fileMappingObject->path()))
{
#ifdef WIN32
    DWORD desiredAccess;
    if (accessMode() >= AccessMode::RDWR) {
        desiredAccess = FILE_MAP_WRITE;
    } else if (accessMode() >= AccessMode::RDONLY) {
        desiredAccess = FILE_MAP_READ;
    } else {
        BOOST_THROW_EXCEPTION(FileError("Illigal access mode WRONLY/0 when viewing file.") <<
            boost::errinfo_file_name(path().string())
        );
    }

    DWORD fileOffsetHigh = offset >> 32;
    DWORD fileOffsetLow = offset & 0xffffffff;

    if ((data = MapViewOfFile(fileMappingObject->intrinsic, desiredAccess, fileOffsetHigh, fileOffsetLow, this->size)) == NULL) {
        BOOST_THROW_EXCEPTION(FileError(getLastErrorMessage()) <<
            boost::errinfo_file_name(path().string())
        );
    }
#endif
}

FileView::FileView(const std::filesystem::path &path, AccessMode accessMode, size_t offset, size_t size) :
    FileView(findOrCreateFileMappingObject(path, accessMode, offset + size), offset, size) {}

FileView::~FileView()
{
#ifdef WIN32
    if (!UnmapViewOfFile(data)) {
        LOG_ERROR("Could not unmap view on file '%s'") % getLastErrorMessage();
    }
#endif
}

void FileView::flush(void *base, size_t size)
{
#ifdef WIN32
    if (!FlushViewOfFile(base, size)) {
        BOOST_THROW_EXCEPTION(FileError(getLastErrorMessage()) <<
            boost::errinfo_file_name(path().string())
        );
    }
#endif
}

std::shared_ptr<FileMapping> FileView::findOrCreateFileMappingObject(std::filesystem::path const &path, AccessMode accessMode, size_t size)
{
    cleanup();

    auto const absolutePath = std::filesystem::absolute(path);

    auto &mappings = mappedFileObjects[absolutePath];

    for (auto weak_fileMappingObject: mappings) {
        if (auto fileMappingObject = weak_fileMappingObject.lock()) {
            if (fileMappingObject->size >= size && fileMappingObject->accessMode() >= accessMode) {
                return fileMappingObject;
            }
        }
    }

    auto fileMappingObject = make_shared<FileMapping>(path, accessMode, size);
    mappings.push_back(fileMappingObject);
    return fileMappingObject;
}

void FileView::cleanup()
{
    for (auto &[key, mappings]: mappedFileObjects) {
        erase_if(mappings, [](auto x) {
            return x.expired();
        });
    }

    erase_if(mappedFileObjects, [](auto x) {
        return x.second.size() == 0;
    });
}




}
