// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Application/Application_forward.hpp"
#include <string>

namespace TTauri {

    /*! Application Delegate.
     * Can be subclasses by the actual application to be called when certain events happen.
     */
class ApplicationDelegate {
public:
    ApplicationDelegate() = default;
    virtual ~ApplicationDelegate() = default;
    ApplicationDelegate(const ApplicationDelegate&) = delete;
    ApplicationDelegate& operator=(const ApplicationDelegate&) = delete;
    ApplicationDelegate(ApplicationDelegate&&) = delete;
    ApplicationDelegate& operator=(ApplicationDelegate&&) = delete;

    /*! Called when an application name is needed.
     */
    virtual std::string applicationName() const noexcept = 0;

    /*! Called right before the application loop is started.
     */
    virtual void startingLoop() = 0;

    /*! Called right after the last window is closed
     */
    virtual void lastWindowClosed() = 0;
};

}
