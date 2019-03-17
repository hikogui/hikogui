//
//  Application.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "utils.hpp"

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
        /*! Called right after the constructor.
         */
        virtual void initialize() = 0;

        /*! Called right before the application loop is started.
         */
        virtual void startingLoop() = 0;
    };

    struct Error : virtual boost::exception, virtual std::exception {};
    struct ResourceDirError : virtual Error {};

    /*! Application delegate
     */
    const std::shared_ptr<Delegate> delegate;

    bool initialized = false;
    bool loopStarted = false;

    /*! Directory where resources are located.
     */
    boost::filesystem::path resourceDir;

    /*! Constructor
     * \param applicationDelegate application delegate to use to manage the application
     * \param vulkanExtensions Vulkan extensions that are needed to use Vulkan on this operating system.
     */
    Application(std::shared_ptr<Delegate> applicationDelegate);

    virtual ~Application();

    /*! Initialize the application.
     */
    virtual void initialize();

    virtual void startingLoop();

    /*! Run the operating system's main loop.
     * Must be called after initialize().
     */
    virtual int loop() = 0;

    /*! The shared application / singleton.
     */
    static std::shared_ptr<Application> shared;
};
}
