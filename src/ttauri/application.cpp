// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "application.hpp"
#include "application_delegate.hpp"
#include "static_resource_view.hpp"
#include "logger.hpp"
#include "timer.hpp"
#include "os_detect.hpp"
#include "trace.hpp"
#include "thread.hpp"
#include "metadata.hpp"
#include "text/elusive_icon.hpp"
#include "text/ttauri_icon.hpp"
#include "text/language.hpp"
#include "text/font_book.hpp"
#include "GUI/RenderDoc.hpp"
#include "GUI/theme_book.hpp"
#include "GUI/keyboard_bindings.hpp"
#include "GUI/gui_system_vulkan_win32.hpp"
#include "audio/audio_system.hpp"
#include "audio/audio_system_aggregate.hpp"
#include <memory>

#include "ttauri/metadata.hpp"

namespace tt {

using namespace std;

application::application(
    std::weak_ptr<application_delegate> const &delegate,
    os_handle instance) :
    delegate(delegate), instance(instance)
{
}

application::~application()
{
}

int application::main()
{
    init();

    if (auto delegate_ = delegate.lock()) {
        if (auto exit_value = delegate_->main(*this)) {
            return *exit_value;
        }
    }

    auto exit_value = loop();

    deinit();
    return exit_value;
}

void application::init()
{
    set_thread_name("Main Thread");

    // application is a singleton.
    tt_assert(application::global == nullptr);
    application::global = this;

    if (auto delegate_ = delegate.lock()) {
        delegate_->init(*this);
    }

    init_foundation();
    init_text();
    init_audio();
    init_gui();

    tt_log_info("Started application '{}'.", application_metadata().display_name);
}


void application::init_foundation()
{
    timer::global = std::make_unique<timer>("Maintenance Timer");

    main_thread_id = current_thread_id();

    if (configuration.contains("log-level")) {
        log_level_global = static_cast<log_level>(static_cast<int>(configuration["log-level"]));
    } else {
        log_level_global = make_log_level(log_level::info);
    }
}

void application::init_text()
{
    font_book::global = std::make_unique<font_book>(std::vector<URL>{URL::urlFromSystemfontDirectory()});
    elusive_icons_font_id = font_book::global->register_font(URL("resource:elusiveicons-webfont.ttf"));
    ttauri_icons_font_id = font_book::global->register_font(URL("resource:ttauri_icons.ttf"));

    language::set_preferred_languages(language::read_os_preferred_languages());
}

void application::init_audio()
{
    if (auto delegate_ = delegate.lock()) {
        ttlet audio_system_delegate = delegate_->audio_system_delegate(narrow_cast<application &>(*this));
        if (!audio_system_delegate.expired()) {
            audio_system::global = std::make_shared<audio_system_aggregate>(audio_system_delegate);
            audio_system::global->init();
        }
    }
}

void application::init_gui()
{
    if (auto delegate_ = delegate.lock()) {
        ttlet gui_delegate = delegate_->gui_system_delegate(narrow_cast<application &>(*this));
        if (!gui_delegate.expired()) {
            RenderDoc::global = std::make_unique<RenderDoc>();

            theme_book::global = std::make_unique<theme_book>(std::vector<URL>{URL::urlFromResourceDirectory() / "themes"});
            theme_book::global->set_current_theme_mode(read_os_theme_mode());

            try {
                keyboardBindings.loadSystemBindings();
            } catch (std::exception const &e) {
                tt_log_fatal("Could not load keyboard bindings. \"{}\"", e.what());
            }

            gui_system::global = std::make_unique<gui_system_vulkan_win32>(gui_delegate);
            gui_system::global->init();
        }
    }
}

void application::deinit()
{
    tt_log_info("Stopping application.");

    deinit_gui();
    deinit_audio();
    deinit_text();
    deinit_foundation();

    if (auto delegate_ = delegate.lock()) {
        delegate_->deinit(*this);
    }

    // Remove the singleton.
    tt_assert(application::global == this);
    application::global = nullptr;
}

void application::deinit_foundation()
{
    // Force all timers to finish.
    timer::global->stop();
    timer::global->remove_callback(clock_maintenance_callback);
    timer::global->remove_callback(logger_maintenance_callback);
    timer::global = {};
}

void application::deinit_text()
{
    elusive_icons_font_id = font_id{};
    ttauri_icons_font_id = font_id{};
    font_book::global = {};
}

void application::deinit_audio()
{
    audio_system::global = {};
}

void application::deinit_gui()
{
    gui_system::global = {};
    theme_book::global = {};
    RenderDoc::global = {};
}

} // namespace tt
