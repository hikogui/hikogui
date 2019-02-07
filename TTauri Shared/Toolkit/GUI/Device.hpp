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

enum class DeviceState {
    NO_DEVICE,
    READY_TO_DRAW,
};

/*! A Device that handles a set of windows.
 */
class Device {
private:
    boost::shared_mutex stateMutex;
    DeviceState state;

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

    /*! Find the minimum number of queue families to instantiate for a window.
     * This will give priority for having the Graphics and Present in the same
     * queue family.
     *
     * It is possible this method returns an incomplete queue family set. For
     * example without Present.
     */
    std::vector<std::pair<uint32_t, QueueCapabilities>> findBestQueueFamilyIndices(std::shared_ptr<Window> window);

    /*! Check if this device is a good match for this window.
     *
     * It is possible for a window to be created that is not presentable, in case of a headless-virtual-display,
     * however in this case it may still be able to be displayed by any device.
     *
     * \returns -1 When not viable, 0 when not presentable, postive values for increasing score.
     */
    int score(std::shared_ptr<Window> window);

    /*! Initialise the logical device.
     *
     * \param window is used as prototype for queue allocation.
     */
    void initializeDevice(std::shared_ptr<Window> window);

    void add(std::shared_ptr<Window> window);

    void remove(std::shared_ptr<Window> window);

    /*! Refresh Display.
     *
     * \outTimestamp Number of nanoseconds since system start.
     * \outputTimestamp Number of nanoseconds since system start until the frame will be displayed on the screen.
     */
    void frameUpdate(uint64_t nowTimestamp, uint64_t outputTimestamp);


    Device(Instance *parent, vk::PhysicalDevice physicalDevice);
    ~Device();
};

}}}
