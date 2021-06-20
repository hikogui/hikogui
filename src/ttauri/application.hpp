// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "thread.hpp"
#include "required.hpp"
#include "timer.hpp"
#include "datum.hpp"
#include "GUI/gui_window_size.hpp"
#include <span>
#include <memory>
#include <string>
#include <any>
#include <map>
#include <thread>

namespace tt {
class audio_system;
class font_book;
class theme_book;
class RenderDoc;
class unicode_data;
class application_delegate;
class URL;

/*! A singleton that represents the application.
 * An application should be instantiated in a local variable in main.
 * This will allow the application to destruct application systems in the
 * correct order when main() goes out of scope and before the global variables
 * are destructed.
 *
 */
class application
{
public:
    static inline application *global;

    /*! application delegate
    */
    std::weak_ptr<application_delegate> delegate;

    /** The initial window size for the first application window.
     */
    gui_window_size initial_window_size = gui_window_size::normal;

    /** The global configuration.
    */
    datum configuration;

    /** Thread id of the main thread.
    */
    thread_id main_thread_id;

    /**
     * This function will take ownership of the delegate and delete it
     * during destruction.
     * 
     * @param delegate A pointer to an application delegate.
     * @param arguments A list of command line arguments.
     *                  On posix this is simply each argument,
     *                  for win32 it needs to be processed to process quotes and
     *                  escaping correctly..
     */
    application(
        std::weak_ptr<application_delegate> const &delegate,
        os_handle instance
    );

    /** Start the application.
     */
    virtual int main();

    virtual ~application();
    application(const application &) = delete;
    application &operator=(const application &) = delete;
    application(application &&) = delete;
    application &operator=(application &&) = delete;

    /*! Run the given function on the main thread.
     */
    virtual void run_from_main_loop(std::function<void()> function) = 0;

    /** Exit the main loop and exit the application.
     */
    virtual void exit(int exit_code=0) = 0;

protected:
    /** Two phase construction.
     */
    virtual void init();

    /** Two phase destruction.
     */
    virtual void deinit();

    /*! Run the operating system's main loop.
     * Must be called after init().
     */
    virtual int loop() = 0;

    virtual void init_audio();
    virtual void deinit_audio();
    virtual void init_text();
    virtual void deinit_text();
    virtual void init_gui();
    virtual void deinit_gui();

private:
    os_handle instance;
};

}

