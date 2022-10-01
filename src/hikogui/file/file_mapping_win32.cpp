// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../win32_headers.hpp"

#include "file_mapping.hpp"
#include "../exception.hpp"
#include "../log.hpp"
#include "../memory.hpp"
#include "../utility.hpp"
#include <mutex>

namespace hi { inline namespace v1 {

namespace detail {

class file_mapping_win32 final : public file_mapping_impl {
public:
    file_mapping_win32(std::shared_ptr<file_impl> const &file, std::size_t size) : file_mapping_impl(file)
    {
        DWORD protect;
        if (any(access_mode() & access_mode::read) and any(access_mode() & access_mode::write)) {
            protect = PAGE_READWRITE;
        } else if (any(access_mode() & access_mode::read)) {
            protect = PAGE_READONLY;
        } else {
            throw io_error(std::format("{}: Illegal access mode WRONLY/0 when mapping file.", path().string()));
        }

        DWORD maximum_size_high = this->size >> 32;
        DWORD maximum_size_low = this->size & 0xffffffff;

        if (this->size == 0) {
            mapHandle = nullptr;
        } else {
            if ((mapHandle = CreateFileMappingW(file->_file_handle, NULL, protect, maximumm_size_high, maximum_size_low, nullptr)) ==
                nullptr) {
                throw io_error(std::format("{}: Could not create file mapping. '{}'", path().string(), get_last_error_message()));
            }
        }
    }

file_mapping::file_mapping(std::filesystem::path const &path, access_mode accessMode, std::size_t size) :
    file_mapping(findOrOpenFile(path, accessMode), size)
{
}

file_mapping::~file_mapping()
{
    if (mapHandle != nullptr) {
        if (!CloseHandle(mapHandle)) {
            hi_log_fatal("Could not close file mapping object on file '{}'", get_last_error_message());
        }
    }
}

}
}} // namespace hi::inline v1
