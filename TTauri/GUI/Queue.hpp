//
//  Queue.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-06.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "QueueCapabilities.hpp"
#include <vulkan/vulkan.hpp>

namespace TTauri {
namespace GUI {

class Device;
class Instance;


class Queue {
public:
    vk::Queue intrinsic;
    uint32_t queueFamilyIndex;
    uint32_t queueIndex;
    const QueueCapabilities queueCapabilities;

    Device *device;

    vk::CommandPool commandPool;

    /**
     */
    Queue(Device *device, uint32_t queueFamilyIndex, uint32_t queueIndex, QueueCapabilities queueCapabilities);
    ~Queue();
};

}}
