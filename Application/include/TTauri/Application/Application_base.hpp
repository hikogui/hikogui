// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Application/ApplicationDelegate.hpp"
#include "TTauri/Widgets/globals.hpp"
#include "TTauri/GUI/globals.hpp"
#include "TTauri/GUI/InstanceDelegate.hpp"
#include "TTauri/Draw/globals.hpp"
#include "TTauri/Audio/globals.hpp"
#include "TTauri/Audio/AudioSystemDelegate.hpp"
#include "TTauri/Config/globals.hpp"
#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/URL.hpp"
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
class Application_base : public GUI::InstanceDelegate, Audio::AudioSystemDelegate {
public:
    /*! Application delegate
    */
    std::shared_ptr<ApplicationDelegate> delegate;

    FoundationGlobals i_foundation;
    Config::ConfigGlobals i_config;
    Audio::AudioGlobals i_audio;
    Draw::DrawGlobals i_draw;
    GUI::GUIGlobals i_gui;
    GUI::Widgets::WidgetsGlobals i_widgets;

    bool loopStarted = false;

    Application_base(std::shared_ptr<ApplicationDelegate> applicationDelegate, void *hInstance = nullptr, int nCmdShow = 0);
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
    void lastWindowClosed() override;

    /*! Called when the device list has changed.
    * This can happen when external devices are connected or disconnected.
    */
    void audioDeviceListChanged() override;
};

inline Application_base *_application = nullptr;

}
