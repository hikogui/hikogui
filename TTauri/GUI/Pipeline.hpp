//
//  Pipeline.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "TTauri/utils.hpp"

namespace TTauri { namespace GUI {

class Device;
class Window;

class Pipeline {
public:
    struct Error : virtual boost::exception, virtual std::exception {};

    std::weak_ptr<Window> window;

    Pipeline(const std::shared_ptr<Window> &window);
    virtual ~Pipeline();

    template<typename T>
    std::shared_ptr<T> device() const {
        return lock_dynamic_cast<T>(window.lock()->device);
    }
};

}}
