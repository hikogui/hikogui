//
//  Application.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "Instance.hpp"

#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>

#include <memory>
#include <string>

namespace TTauri {

/*! A singleton that represents the application.
 * There should only be one Application instance.
 * It should be constructed in main() or equivilant and assigned to Application::shared.
 *
 */
class Application {
public:
    /*! Application Delegate.
     * Can be subclasses by the actual application to be called when certain events happen.
     */
    class Delegate {
    public:
        /*! Called when the application needs to be initialized.
         * This is a good place to open the window and setup an application's own objects.
         */
        virtual void initialize() = 0;
    };

    struct Error : virtual boost::exception, virtual std::exception {};

    /*! Application delegate
     */
    const std::shared_ptr<Delegate> delegate;

    /*! Vulkan instance created for this application.
     */
    std::shared_ptr<Instance> instance;

    /*! Directory where resources are located.
     */
    boost::filesystem::path resourceDir;

    /*! Constructor
     * \param applicationDelegate application delegate to use to manage the application
     * \param vulkanExtensions Vulkan extensions that are needed to use Vulkan on this operating system.
     */
    Application(std::shared_ptr<Delegate> applicationDelegate, std::vector<const char *> vulkanExtensions);

    virtual ~Application();

    /*! Open a new window.
     *
     * \param windowDelegate window delegate to use to manage the window.
     * \param title Title for the new window
     * \return the window that was created
     */
    virtual std::shared_ptr<Window> createWindow(std::shared_ptr<Window::Delegate> windowDelegate, const std::string &title) = 0;

    /*! Initialize the application.
     * Must be called from main after constructing the Application.
     */
    virtual void initialize();

    /*! Run the operating system's main loop.
     * Must be called after initialize().
     */
    virtual int loop() = 0;

    /*! The shared application / singleton.
     */
    static std::shared_ptr<Application> shared;
};
}
