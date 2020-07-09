// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Application_forward.hpp"
#include "datum.hpp"
#include <string>
#include <vector>

namespace tt {

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

    /*! Return the possible command line argument options.
     */
    virtual datum configuration(std::vector<std::string> arguments) const noexcept = 0;

    /** Initialize the application.
     * Called right before the application loop is started.
     *
     * @return true to start the loop, false to exit the application.
     */
    virtual bool initializeApplication() = 0;

    /*! Called right after the last window is closed
     */
    virtual void lastWindowClosed() = 0;

    /*! Called when the device list has changed.
    * This can happen when external devices are connected or disconnected.
    */
    virtual void audioDeviceListChanged() = 0;

};

}
