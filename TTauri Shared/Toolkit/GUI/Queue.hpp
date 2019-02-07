//
//  Queue.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-06.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include "QueueCapabilities.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

class Device;
class Instance;


class Queue {
public:
    vk::Queue intrinsic;
    const QueueCapabilities queueCapabilities;

    Device *device;
    Instance *instance;

    /**
     */
    Queue(Device *device, uint32_t queueFamilyIndex, uint32_t queueIndex, QueueCapabilities queueCapabilities);
    ~Queue();
};

}}}
