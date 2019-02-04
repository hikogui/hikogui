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
#include "Window.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

/** Vulkan Device controller.
 * Manages Vulkan device and a set of Windows.
 */
class Device {
    std::vector<std::shared_ptr<Window>> windows;
};

}}}
