//
//  Queue.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-06.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Queue.hpp"

#include "Device_vulkan.hpp"
#include "Instance.hpp"

namespace TTauri {
namespace GUI {

Queue::Queue(const std::shared_ptr<Device> &device, uint32_t queueFamilyIndex, uint32_t queueIndex, QueueCapabilities queueCapabilities) :
    device(device),
    intrinsic(std::dynamic_pointer_cast<Device_vulkan>(device)->intrinsic.getQueue(queueFamilyIndex, queueIndex)),
    queueIndex(queueIndex),
    queueFamilyIndex(queueFamilyIndex),
    queueCapabilities(queueCapabilities)
{
    auto commandPoolCreateInfo = vk::CommandPoolCreateInfo(
        vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        queueFamilyIndex);

    commandPool = lock_dynamic_cast<Device_vulkan>(this->device)->intrinsic.createCommandPool(commandPoolCreateInfo);
}

Queue::~Queue()
{
    lock_dynamic_cast<Device_vulkan>(device)->intrinsic.destroy(commandPool);
}

}}
