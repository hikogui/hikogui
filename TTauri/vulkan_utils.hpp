//
//  vulkan_utils.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-05.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/exception/all.hpp>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

namespace TTauri {

struct VulkanError: virtual boost::exception, virtual std::exception {};

static inline void setExtensionNames(vk::InstanceCreateInfo &instanceCreateInfo, const std::vector<const char *> &extensions)
{
    instanceCreateInfo.setEnabledExtensionCount(boost::numeric_cast<uint32_t>(extensions.size()));
    instanceCreateInfo.setPpEnabledExtensionNames(extensions.data());
}

static inline void setExtensionNames(vk::DeviceCreateInfo &deviceCreateInfo, const std::vector<const char *> &extensions)
{
    deviceCreateInfo.setEnabledExtensionCount(boost::numeric_cast<uint32_t>(extensions.size()));
    deviceCreateInfo.setPpEnabledExtensionNames(extensions.data());
}

static inline void setLayerNames(vk::InstanceCreateInfo &instanceCreateInfo, const std::vector<const char *> &layers)
{
    instanceCreateInfo.setEnabledLayerCount(boost::numeric_cast<uint32_t>(layers.size()));
    instanceCreateInfo.setPpEnabledLayerNames(layers.data());
}

static inline void setLayerNames(vk::DeviceCreateInfo &deviceCreateInfo, const std::vector<const char *> &layers)
{
    deviceCreateInfo.setEnabledLayerCount(boost::numeric_cast<uint32_t>(layers.size()));
    deviceCreateInfo.setPpEnabledLayerNames(layers.data());
}

static inline void setQueueCreateInfos(vk::DeviceCreateInfo &deviceCreateInfo, const std::vector<vk::DeviceQueueCreateInfo> &createInfos)
{
    deviceCreateInfo.setQueueCreateInfoCount(boost::numeric_cast<uint32_t>(createInfos.size()));
    deviceCreateInfo.setPQueueCreateInfos(createInfos.data());
}

bool meetsRequiredLimits(const vk::PhysicalDevice &physicalDevice, const vk::PhysicalDeviceLimits &requiredLimits);

bool hasRequiredFeatures(const vk::PhysicalDevice &physicalDevice, const vk::PhysicalDeviceFeatures &requiredFeatures);

}
