// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/win32_headers.hpp"

#include "file.hpp"

namespace hi { inline namespace v1 {

namespace detail {

class file_win32 final : public file_impl {
public:
    ~file_win32();
    file_win32(std::filesystem::path const &path, hi::access_mode access_mode);
    
    [[nodiscard]] bool closed() noexcept override
    {
        return _file_handle == nullptr;
    }

    [[nodiscard]] HANDLE file_handle() const noexcept
    {
        return _file_handle;
    }
    
    void flush() override;
    void close() override;
    [[nodiscard]] std::size_t size() const override;
    std::size_t seek(ssize_t offset, seek_whence whence) override;
    void rename(std::filesystem::path const& destination, bool overwrite_existing) override;
    void write(void const *data, std::size_t size) override;
    [[nodiscard]] std::size_t read(void *data, std::size_t size) override;

private:
    HANDLE _file_handle = nullptr;
};

}

}}
