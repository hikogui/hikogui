// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Device_forward.hpp"
#include "Window_forward.hpp"
#include "TTauri/required.hpp"
#include <boost/exception/exception.hpp>
#include <memory>

namespace TTauri::GUI {

class Pipeline_base {
public:
    struct Error : virtual boost::exception, virtual std::exception {};

    Window const &window;

    Pipeline_base(Window const &window);

    virtual ~Pipeline_base() = default;
    Pipeline_base(const Pipeline_base &) = delete;
    Pipeline_base &operator=(const Pipeline_base &) = delete;
    Pipeline_base(Pipeline_base &&) = delete;
    Pipeline_base &operator=(Pipeline_base &&) = delete;

    Device const &device() const;

};

}
