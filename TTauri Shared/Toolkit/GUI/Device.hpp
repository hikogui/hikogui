//
//  Device.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "vulkan_utils.hpp"
#include "Window.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

/** Vulkan Device controller.
 * Manages Vulkan device and a set of Windows.
 */
class Device {
public:
    std::vector<const char *> extensionNames;
    vk::ApplicationInfo applicationInfo;
    vk::Instance instance;

    std::vector<std::shared_ptr<Window>> windows;

    void add(std::shared_ptr<Window> window) {
        windows.push_back(window);
    }

    Device(const std::vector<const char *> &extensions);
    ~Device();
};

}}}
