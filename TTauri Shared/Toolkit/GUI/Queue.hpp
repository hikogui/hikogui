//
//  Queue.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-06.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <vulkan/vulkan.hpp>

namespace TTauri {
namespace Toolkit {
namespace GUI {

class Device;
class Instance;

class Queue {
public:
    Device *device;
    Instance *instance;

    Queue(Device *device);
    ~Queue();
};

}}}
