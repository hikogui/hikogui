// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <vulkan/vulkan.hpp>
#include <cstdint>

export module hikogui_GFX : gfx_queue;

export namespace hi::inline v1 {

struct gfx_queue_vulkan {
    uint32_t family_queue_index;
    uint32_t queue_index;
    vk::QueueFlags flags;
    vk::Queue queue;
    vk::CommandPool command_pool;
};

} // namespace hi::inline v1
