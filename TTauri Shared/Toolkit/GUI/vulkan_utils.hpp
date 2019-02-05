//
//  vulkan_utils.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-05.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <cstdint>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/exception/all.hpp>
#include <vulkan/vulkan.hpp>

namespace TTauri {
namespace Toolkit {
namespace GUI {

struct VulkanError: virtual boost::exception, virtual std::exception {};

static inline void setExtensionNames(vk::InstanceCreateInfo &instanceCreateInfo, const std::vector<const char *> &extensions)
{
    instanceCreateInfo.setEnabledExtensionCount(boost::numeric_cast<uint32_t>(extensions.size()));
    instanceCreateInfo.setPpEnabledExtensionNames(extensions.data());
}

/*! Check if the Vulkan driver includes all required extensions.
 */
void checkRequiredExtensions(const std::vector<const char *> &requiredExtensions);

/*! Create a list of PhysicalDevices sorted best to worst.
 * All devices in the resulting list will have all the required features and meet all the required limits.
 */
std::vector<vk::PhysicalDevice> findBestPhysicalDevices(const vk::Instance instance, const vk::PhysicalDeviceFeatures &requiredFeatures, const vk::PhysicalDeviceLimits &requiredLimits, const vk::QueueFlags &requiredQueueFlags);

}}}
