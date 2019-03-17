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

Queue::Queue(Device *device, uint32_t queueFamilyIndex, uint32_t queueIndex, QueueCapabilities queueCapabilities) :
    device(device),
    intrinsic(checked_dynamic_cast<Device_vulkan *>(device)->intrinsic.getQueue(queueFamilyIndex, queueIndex)),
    queueIndex(queueIndex),
    queueFamilyIndex(queueFamilyIndex),
    queueCapabilities(queueCapabilities)
{
    auto commandPoolCreateInfo = vk::CommandPoolCreateInfo(
        vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        queueFamilyIndex);

    commandPool = checked_dynamic_cast<Device_vulkan *>(device)->intrinsic.createCommandPool(commandPoolCreateInfo);
}

Queue::~Queue()
{
    checked_dynamic_cast<Device_vulkan *>(device)->intrinsic.destroy(commandPool);
}

}}
