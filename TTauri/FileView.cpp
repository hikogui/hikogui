// Copyright 2019 Pokitec
// All rights reserved.

#include "FileView.hpp"
#include "required.hpp"
#include "logging.hpp"
#include "utils.hpp"

namespace TTauri {

std::map<std::filesystem::path, std::vector<std::weak_ptr<FileMapping>>> FileView::mappedFileObjects;

FileView::FileView(std::shared_ptr<FileMapping> const& fileMappingObject, size_t offset, size_t size) :
    fileMappingObject(fileMappingObject),
    offset(offset)
{
    if (size == 0) {
        size = std::filesystem::file_size(fileMappingObject->path());
    }

#ifdef WIN32
    DWORD desiredAccess;
    if (accessMode() >= AccessMode::RDWR) {
        desiredAccess = FILE_MAP_WRITE;
    }
    else if (accessMode() >= AccessMode::RDONLY) {
        desiredAccess = FILE_MAP_READ;
    }
    else {
        BOOST_THROW_EXCEPTION(FileError("Illigal access mode WRONLY/0 when viewing file.") <<
            boost::errinfo_file_name(path().string())
        );
    }

    DWORD fileOffsetHigh = offset >> 32;
    DWORD fileOffsetLow = offset & 0xffffffff;

    void *data;
    if ((data = MapViewOfFile(fileMappingObject->intrinsic, desiredAccess, fileOffsetHigh, fileOffsetLow, size)) == NULL) {
        BOOST_THROW_EXCEPTION(FileError(getLastErrorMessage()) <<
            boost::errinfo_file_name(path().string())
        );
    }

    bytes = gsl::span<std::byte>(static_cast<std::byte *>(data), size);
#endif
}

FileView::FileView(const std::filesystem::path & path, AccessMode accessMode, size_t offset, size_t size) :
    FileView(findOrCreateFileMappingObject(path, accessMode, offset + size), offset, size) {}

FileView::~FileView()
{
#ifdef WIN32
    void *data = bytes.data();
    if (!UnmapViewOfFile(data)) {
        LOG_ERROR("Could not unmap view on file '%s'", getLastErrorMessage());
    }
#endif
}

void FileView::flush(void* base, size_t size)
{
#ifdef WIN32
    if (!FlushViewOfFile(base, size)) {
        BOOST_THROW_EXCEPTION(FileError(getLastErrorMessage()) <<
            boost::errinfo_file_name(path().string())
        );
    }
#endif
}

std::shared_ptr<FileMapping> FileView::findOrCreateFileMappingObject(std::filesystem::path const& path, AccessMode accessMode, size_t size)
{
    cleanup();

    let absolutePath = std::filesystem::absolute(path);

    auto& mappings = mappedFileObjects[absolutePath];

    for (auto weak_fileMappingObject : mappings) {
        if (auto fileMappingObject = weak_fileMappingObject.lock()) {
            if (fileMappingObject->size >= size && fileMappingObject->accessMode() >= accessMode) {
                return fileMappingObject;
            }
        }
    }

    auto fileMappingObject = std::make_shared<FileMapping>(path, accessMode, size);
    mappings.push_back(fileMappingObject);
    return fileMappingObject;
}

void FileView::cleanup()
{
    for (auto& [key, mappings] : mappedFileObjects) {
        erase_if(mappings, [](auto x) {
            return x.expired();
        });
    }

    erase_if(mappedFileObjects, [](auto x) {
        return x.second.size() == 0;
    });
}

}