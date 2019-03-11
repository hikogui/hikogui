//
//  Device.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Instance.hpp"
#include "vulkan_utils.hpp"
#include "TTauri/Logging.hpp"
#include <chrono>

namespace TTauri {
namespace GUI {

using namespace std;

static bool hasRequiredExtensions(const std::vector<const char *> &requiredExtensions)
{
    auto availableExtensions = std::unordered_set<std::string>();
    for (auto availableExtensionProperties : vk::enumerateInstanceExtensionProperties()) {
        availableExtensions.insert(std::string(availableExtensionProperties.extensionName));
    }

    for (auto requiredExtension : requiredExtensions) {
        if (availableExtensions.count(requiredExtension) == 0) {
            return false;
        }
    }
    return true;
}

Instance::Instance(const std::vector<const char *> &extensionNames) :
    requiredExtensions(extensionNames), requiredLayers(), requiredFeatures(), requiredLimits()
{
    applicationInfo = vk::ApplicationInfo(
        "TTauri App", VK_MAKE_VERSION(0, 1, 0),
        "TTauri Engine", VK_MAKE_VERSION(0, 1, 0),
        VK_API_VERSION_1_0
    );

    requiredExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    if (!hasRequiredExtensions(requiredExtensions)) {
        BOOST_THROW_EXCEPTION(Instance::Error());
    }

    auto instanceCreateInfo = vk::InstanceCreateInfo(
        vk::InstanceCreateFlags(),
        &applicationInfo
    );
    setExtensionNames(instanceCreateInfo, requiredExtensions);

#if defined(_WIN32) && !defined(NDEBUG)
    requiredLayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif
    setLayerNames(instanceCreateInfo, requiredLayers);

    LOG_INFO("Creating Vulkan instance.");
    intrinsic = vk::createInstance(instanceCreateInfo);

#if (VK_HEADER_VERSION == 97)
    loader = vk::DispatchLoaderDynamic(intrinsic);
#else
    loader = vk::DispatchLoaderDynamic(intrinsic, vkGetInstanceProcAddr);
#endif
    for (auto _physicalDevice: intrinsic.enumeratePhysicalDevices()) {
        auto physicalDevice = make_shared<Device>(this, _physicalDevice);
        physicalDevices.push_back(physicalDevice);
    }

    maintanceThreadInstance = std::thread(Instance::maintanceThread, this);
}

bool Instance::add(std::shared_ptr<Window> window)
{
    int bestScore = -1;
    shared_ptr<Device> bestDevice;
    
    for (auto physicalDevice: physicalDevices) {
        auto score = physicalDevice->score(window);
        LOG_INFO("Device has score=%i.") % score;

        if (score >= bestScore) {
            bestScore = score;
            bestDevice = physicalDevice;
        }
    }

    switch (bestScore) {
    case -1:
        return false;
    case 0:
        fprintf(stderr, "Could not really find a device that can present this window.");
        /* FALLTHROUGH */
    default:
        bestDevice->add(window);
    }
    return true;
}

Instance::~Instance()
{
    state = InstanceState::STOPPING;
    while (state != InstanceState::STOPPED) {
        std::this_thread::sleep_for(std::chrono::milliseconds(67));
    };

    intrinsic.destroy();
}


void Instance::updateAndRender(uint64_t nowTimestamp, uint64_t outputTimestamp, bool blockOnVSync)
{
    for (auto physicalDevice: physicalDevices) {
        physicalDevice->updateAndRender(nowTimestamp, outputTimestamp, blockOnVSync);
    }
}

void Instance::maintance()
{
    for (auto device: physicalDevices) {
        device->maintance();
    }
}

void Instance::maintanceThread(Instance *self)
{
    self->state = InstanceState::RUNNING;

    while (self->state == InstanceState::RUNNING) {
        std::this_thread::sleep_for(std::chrono::milliseconds(67));
        self->maintance();
    }
    self->state = InstanceState::STOPPED;
}

}}
