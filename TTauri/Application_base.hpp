// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "utils.hpp"
#include "ApplicationDelegate.hpp"
#include "URL.hpp"
#include <boost/exception/all.hpp>
#include <gsl/gsl>
#include <filesystem>
#include <memory>
#include <string>
#include <any>
#include <map>

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
    std::shared_ptr<ApplicationDelegate> delegate;

    bool loopStarted = false;

    /*! Directory where resources are located.
     */
    URL resourceLocation;

    Application_base() = default;
    ~Application_base() = default;
    Application_base(const Application_base &) = delete;
    Application_base &operator=(const Application_base &) = delete;
    Application_base(Application_base &&) = delete;
    Application_base &operator=(Application_base &&) = delete;

    /*! Initialize the application.
     */
    virtual void initialize(std::shared_ptr<ApplicationDelegate> applicationDelegate);

    /*! Called right before a loop is started.
     */
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
