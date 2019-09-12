// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/ApplicationDelegate.hpp"
#include "TTauri/Time/globals.hpp"
#include "TTauri/Required/required.hpp"
#include "TTauri/Required/URL.hpp"
#include <gsl/gsl>
#include <memory>
#include <string>
#include <any>
#include <map>
#include <thread>

namespace TTauri {

/*! A singleton that represents the application.
 * An Application should be instantiated in a local variable in main.
 * This will allow the appliation to destruct Application systems in the
 * correct order when main() goes out of scope and before the global varaibles
 * are destructed.
 *
 */
class Application_base {
public:
    /*! Application delegate
     */
    std::shared_ptr<ApplicationDelegate> delegate;

    bool loopStarted = false;

    Application_base(std::shared_ptr<ApplicationDelegate> applicationDelegate);
    virtual ~Application_base();
    Application_base(const Application_base &) = delete;
    Application_base &operator=(const Application_base &) = delete;
    Application_base(Application_base &&) = delete;
    Application_base &operator=(Application_base &&) = delete;

    /*! Run the given function on the main thread.
     */
    virtual void runOnMainThread(std::function<void()> function) = 0;

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

inline Application_base *_application = nullptr;

}
