//
//  Device.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-06.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <unordered_set>
#include <vulkan/vulkan.hpp>

#include "Window.hpp"
#include "Queue.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

class Instance;

/*! A Device that handles a set of windows.
 */
class Device {
public:
    vk::PhysicalDevice physicalIntrinsic;
    vk::Device intrinsic;

    Instance *instance;

    /*! A set of queues to send commands to.
     * These queue objects may be shared.
     *
     * ASSUMPTION: A single presentQueue can be used by all Windows on this Device.
     */
    std::shared_ptr<Queue> graphicQueue;
    std::shared_ptr<Queue> computeQueue;
    std::shared_ptr<Queue> presentQueue;

    std::unordered_set<std::shared_ptr<Window>> windows;

    std::unordered_set<uint32_t> findBestQueueFamilyIndices(std::shared_ptr<Window> window);

    /*! Check if this device is a good match for this window.
     *
     * It is possible for a window to be created that is not presentable, in case of a headless-virtual-display,
     * however in this case it may still be able to be displayed by any device.
     *
     * \returns -1 When not viable, 0 when not presentable, postive values for higher priority.
     */
    int deviceScore(std::shared_ptr<Window> window);

    void initializeDevice();

    void add(std::shared_ptr<Window> window);

    void remove(std::shared_ptr<Window> window);

    Device(Instance *parent, vk::PhysicalDevice physicalDevice);
    ~Device();
};

}}}
