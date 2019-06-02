// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "utils.hpp"
#include "ApplicationDelegate.hpp"
#include <boost/exception/all.hpp>
#include <filesystem>
#include <memory>
#include <string>

namespace TTauri {

/*! A singleton that represents the application.
 * There should only be one Application instance.
 * It should be constructed in main() or equivilant and assigned to Application::shared.
 *
 */
class Application_base {
public:

    struct Error : virtual boost::exception, virtual std::exception {};
    struct ResourceDirError : virtual Error {};

    /*! Application delegate
     */
    const std::shared_ptr<ApplicationDelegate> delegate;

    bool loopStarted = false;

    /*! Directory where resources are located.
     */
    std::filesystem::path resourceDir;

    /*! Constructor
     * \param applicationDelegate application delegate to use to manage the application
     * \param vulkanExtensions Vulkan extensions that are needed to use Vulkan on this operating system.
     */
    Application_base(std::shared_ptr<ApplicationDelegate> applicationDelegate);

    virtual ~Application_base() = default;

    Application_base(const Application_base &) = delete;
    Application_base &operator=(const Application_base &) = delete;
    Application_base(Application_base &&) = delete;
    Application_base &operator=(Application_base &&) = delete;

    /*! Initialize the application.
     */
    virtual void initialize() {}

    virtual void startingLoop();

    /*! Run the operating system's main loop.
     * Must be called after initialize().
     */
    virtual int loop() = 0;

    /*! Called by the GUI when the last window was closed.
    */
    virtual void lastWindowClosed();
};
}
