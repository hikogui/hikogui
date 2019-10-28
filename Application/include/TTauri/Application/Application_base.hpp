// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/config.hpp"
#include "TTauri/Application/ApplicationDelegate.hpp"
#if defined(TTAURI_AUDIO_ENABLED)
#include "TTauri/Audio/globals.hpp"
#include "TTauri/Audio/AudioSystemDelegate.hpp"
#endif
#if defined(TTAURI_GUI_ENABLED)
#include "TTauri/Widgets/globals.hpp"
#include "TTauri/GUI/globals.hpp"
#include "TTauri/GUI/InstanceDelegate.hpp"
#endif
#if defined(TTAURI_CONFIG_ENABLED)
#include "TTauri/Config/globals.hpp"
#endif
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

class Application_base_dummy {};

/*! A singleton that represents the application.
 * An Application should be instantiated in a local variable in main.
 * This will allow the appliation to destruct Application systems in the
 * correct order when main() goes out of scope and before the global varaibles
 * are destructed.
 *
 */
class Application_base : public Application_base_dummy
#if defined(TTAURI_GUI_ENABLED)
    , GUI::InstanceDelegate
#endif
#if defined(TTAURI_AUDIO_ENABLED)
    , Audio::AudioSystemDelegate
#endif
{
public:
    /*! Application delegate
    */
    std::shared_ptr<ApplicationDelegate> delegate;

    FoundationGlobals i_foundation;
#if defined(TTAURI_CONFIG_ENABLED)
    Config::ConfigGlobals i_config;
#endif
#if defined(TTAURI_AUDIO_ENABLED)
    Audio::AudioGlobals i_audio;
#endif
#if defined(TTAURI_GUI_ENABLED)
    GUI::GUIGlobals i_gui;
    GUI::Widgets::WidgetsGlobals i_widgets;
#endif
    int i_dummy;

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

#if defined(TTAURI_GUI_ENABLED)
    /*! Called by the GUI when the last window was closed.
    */
    void lastWindowClosed() override;
#endif

#if defined(TTAURI_AUDIO_ENABLED)
    /*! Called when the device list has changed.
    * This can happen when external devices are connected or disconnected.
    */
    void audioDeviceListChanged() override;
#endif
};

inline Application_base *_application = nullptr;

}
