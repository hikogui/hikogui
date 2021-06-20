// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "application.hpp"
#include "application_delegate.hpp"
#include "static_resource_view.hpp"
#include "logger.hpp"
#include "timer.hpp"
#include "architecture.hpp"
#include "trace.hpp"
#include "thread.hpp"
#include "metadata.hpp"
#include "text/elusive_icon.hpp"
#include "text/ttauri_icon.hpp"
#include "text/language.hpp"
#include "text/font_book.hpp"
#include "GUI/RenderDoc.hpp"
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

    init_gui();

    tt_log_info("Started application '{}'.", application_metadata().display_name);
}

void application::init_gui()
{
    if (auto delegate_ = delegate.lock()) {
        ttlet gui_delegate = delegate_->gui_system_delegate(narrow_cast<application &>(*this));
        if (!gui_delegate.expired()) {
            RenderDoc::global = std::make_unique<RenderDoc>();
           
            gui_system::global = std::make_unique<gui_system_vulkan_win32>(gui_delegate, instance);
            gui_system::global->init();
        }
    }
}

void application::deinit()
{
    tt_log_info("Stopping application.");

    deinit_gui();

    if (auto delegate_ = delegate.lock()) {
        delegate_->deinit(*this);
    }

    // Remove the singleton.
    tt_assert(application::global == this);
    application::global = nullptr;
}

void application::deinit_gui()
{
    gui_system::global = {};
    RenderDoc::global = {};
}

} // namespace tt
