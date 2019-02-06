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

Queue::Queue(Device *device) :
    device(device), instance(device->instance)
{
}

Queue::~Queue()
{
}

}}}
