// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_system_vulkan.hpp"
#include <span>

namespace tt {

class gfx_system_vulkan_win32 final: public gfx_system_vulkan {
public:
    gfx_system_vulkan_win32(std::weak_ptr<gfx_system_delegate> const &delegate, os_handle instance);
    ~gfx_system_vulkan_win32();

    gfx_system_vulkan_win32(const gfx_system_vulkan_win32 &) = delete;
    gfx_system_vulkan_win32 &operator=(const gfx_system_vulkan_win32 &) = delete;
    gfx_system_vulkan_win32(gfx_system_vulkan_win32 &&) = delete;
    gfx_system_vulkan_win32 &operator=(gfx_system_vulkan_win32 &&) = delete;

    [[nodiscard]] std::unique_ptr<gfx_surface> make_surface(void *os_window) const noexcept override;

};

}
