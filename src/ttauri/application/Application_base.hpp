// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Application/ApplicationDelegate.hpp"
#include "ttauri/audio/globals.hpp"
#include "ttauri/audio/AudioSystemDelegate.hpp"
#include "ttauri/text/globals.hpp"
#include "ttauri/widgets/globals.hpp"
#include "TTauri/GUI/globals.hpp"
#include "TTauri/GUI/GUISystemDelegate.hpp"
#include "ttauri/foundation/globals.hpp"
#include "ttauri/foundation/required.hpp"
#include "ttauri/foundation/URL.hpp"
#include <nonstd/span>
#include <memory>
#include <string>
#include <any>
#include <map>
#include <thread>

namespace tt {

class Application_base_dummy {};

/*! A singleton that represents the application.
 * An Application should be instantiated in a local variable in main.
 * This will allow the application to destruct Application systems in the
 * correct order when main() goes out of scope and before the global variables
 * are destructed.
 *
 */
class Application_base : public Application_base_dummy
    , GUISystemDelegate
    , AudioSystemDelegate
{
public:
    /*! Application delegate
    */
    std::shared_ptr<ApplicationDelegate> delegate;

    /*! Command line arguments.
     */
    std::vector<std::string> arguments;

    Application_base(std::shared_ptr<ApplicationDelegate> applicationDelegate, std::vector<std::string> const &arguments, void *hInstance = nullptr, int nCmdShow = 0);
    virtual ~Application_base();
    Application_base(const Application_base &) = delete;
    Application_base &operator=(const Application_base &) = delete;
    Application_base(Application_base &&) = delete;
    Application_base &operator=(Application_base &&) = delete;

    /*! Run the given function on the main thread.
     */
    virtual void runOnMainThread(std::function<void()> function) = 0;

    /*! Run the operating system's main loop.
     * Must be called after initialize().
     */
    virtual int loop() = 0;

    /*! Called when the device list has changed.
    * This can happen when external devices are connected or disconnected.
    */
    void audioDeviceListChanged() override;

protected:
    /*! Called right before a loop is started.
    */
    virtual bool startingLoop();
};

inline Application_base *_application = nullptr;

}
