//
//  Queue.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-06.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Queue.hpp"
#include "Device.hpp"
#include "Instance.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

Queue::Queue(Device *device, uint32_t queueFamilyIndex, uint32_t queueIndex, QueueCapabilities queueCapabilities) :
    device(device), instance(device->instance), intrinsic(device->intrinsic.getQueue(queueFamilyIndex, queueIndex)),
    queueIndex(queueIndex), queueFamilyIndex(queueFamilyIndex),
    queueCapabilities(queueCapabilities)
{
}

Queue::~Queue()
{
}

}}}
