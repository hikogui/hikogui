// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Device_forward.hpp"
#include "Window_forward.hpp"
#include <boost/exception/exception.hpp>
#include <memory>

namespace TTauri::GUI {

class Pipeline_base {
public:
    struct Error : virtual boost::exception, virtual std::exception {};

    std::weak_ptr<Window> window;

    Pipeline_base(const std::shared_ptr<Window> window);

    virtual ~Pipeline_base() = default;
    Pipeline_base(const Pipeline_base &) = delete;
    Pipeline_base &operator=(const Pipeline_base &) = delete;
    Pipeline_base(Pipeline_base &&) = delete;
    Pipeline_base &operator=(Pipeline_base &&) = delete;

    std::shared_ptr<Device> device() const;
};

}
