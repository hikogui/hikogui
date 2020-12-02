// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "application_delegate.hpp"
#include "audio/audio_system_delegate.hpp"
#include "GUI/gui_system_delegate.hpp"
#include "GUI/gui_system_globals.hpp"
#include "required.hpp"
#include "URL.hpp"
#include "timer.hpp"
#include <span>
#include <memory>
#include <string>
#include <any>
#include <map>
#include <thread>

namespace tt {
class audio_system;
class FontBook;
class ThemeBook;
class RenderDoc;
class UnicodeData;

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

    /*! Command line arguments.
     */
    std::vector<std::string> arguments;

    /** The global configuration.
    */
    datum configuration;

    /** The system timezone.
    */
    date::time_zone const *timeZone = nullptr;

    /** Thread id of the main thread.
    */
    std::thread::id mainThreadID;

    std::atomic<bool> inLoop;

    std::unique_ptr<FontBook> fonts;
    std::unique_ptr<ThemeBook> themes;
    std::unique_ptr<RenderDoc> renderDoc;
    std::unique_ptr<UnicodeData> unicodeData;

    /**
     * This function will take ownership of the delegate and delete it
     * during destruction.
     * 
     * @param delegate A pointer to an application delegate.
     */
    application(
        std::weak_ptr<application_delegate> const &delegate,
        std::vector<std::string> const &arguments
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

    /** Quit the main loop and exit the application.
     */
    virtual void quit() = 0;

    /** Get the data of a static resource.
     * These are resources that where linked into the exectuable.
     *
     * @param key Name of the resource.
     * @return A span to the constant byte array.
     * @exception key_error Thrown when static resource could not be found.
     */
    std::span<std::byte const> getStaticResource(std::string const &key);

protected:
    std::unordered_map<std::string,std::span<std::byte const>> staticResources;

    /** Add static resource.
     * This function should only be called on resources that are linked into the executable
     * and therefor only be called by the application class.
     *
     * @param key Name of the resource.
     * @param value A span to the constant byte array
     */
    void addStaticResource(std::string const &key, std::span<std::byte const> value) noexcept;

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

    virtual void foundationStart();
    virtual void foundationStop();
    virtual void audioStart();
    virtual void audioStop();
    virtual void textStart();
    virtual void textStop();
    virtual void GUIStart();
    virtual void GUIStop();

private:
    typename timer::callback_ptr_type logger_maintenance_callback;
    typename timer::callback_ptr_type clock_maintenance_callback;
};

}

