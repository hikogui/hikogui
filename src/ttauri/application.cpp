// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "application.hpp"
#include "application_delegate.hpp"
#include "static_resource_view.hpp"
#include "logger.hpp"
#include "timer.hpp"
#include "os_detect.hpp"
#include "version.hpp"
#include "trace.hpp"
#include "thread.hpp"
#include "text/ElusiveIcons.hpp"
#include "text/TTauriIcons.hpp"
#include "text/language.hpp"
#include "text/font_book.hpp"
#include "text/unicode_data.hpp"
#include "GUI/RenderDoc.hpp"
#include "GUI/theme_book.hpp"
#include "GUI/KeyboardBindings.hpp"
#include "GUI/gui_system_vulkan_win32.hpp"
#include "audio/audio_system.hpp"
#include "audio/audio_system_aggregate.hpp"
#include <memory>

#include "data/unicode_data.bin.inl"
#include "data/elusiveicons-webfont.ttf.inl"
#include "data/TTauriIcons.ttf.inl"
#include "ttauri/GUI/PipelineImage.vert.spv.inl"
#include "ttauri/GUI/PipelineImage.frag.spv.inl"
#include "ttauri/GUI/PipelineFlat.vert.spv.inl"
#include "ttauri/GUI/PipelineFlat.frag.spv.inl"
#include "ttauri/GUI/PipelineBox.vert.spv.inl"
#include "ttauri/GUI/PipelineBox.frag.spv.inl"
#include "ttauri/GUI/PipelineSDF.vert.spv.inl"
#include "ttauri/GUI/PipelineSDF.frag.spv.inl"
#include "ttauri/GUI/PipelineToneMapper.vert.spv.inl"
#include "ttauri/GUI/PipelineToneMapper.frag.spv.inl"

namespace tt {

using namespace std;

application::application(std::weak_ptr<application_delegate> const &delegate, std::vector<std::string> const &arguments) :
    delegate(delegate)
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
        application_version.name = delegate_->application_name(narrow_cast<application &>(*this));
        configuration = delegate_->configuration(narrow_cast<application &>(*this), arguments);
    }

    init_foundation();
    init_text();
    init_audio();
    init_gui();

    LOG_INFO("Started application '{}'.", application_version.name);
}


void application::init_foundation()
{
    timer::global = std::make_unique<timer>("Maintenance Timer");

    main_thread_id = current_thread_id();

    logger.minimum_log_level = static_cast<log_level>(static_cast<int>(configuration["log-level"]));

    // The logger is the first object that will use the timezone database.
    // So we will initialize it here.
#if USE_OS_TZDB == 0
    ttlet tzdata_location = URL::urlFromResourceDirectory() / "tzdata";
    date::set_install(tzdata_location.nativePath());
#endif
    try {
        timeZone = date::current_zone();
    } catch (std::runtime_error const &e) {
        LOG_ERROR("Could not get the current time zone, all times shown as UTC: '{}'", tt::to_string(e));
    }

    // First we need a clock, it is used by almost any other service.
    // It will immediately be synchronized, but inaccurately, it will take a while to become
    // more accurate, but we don't want to block here.
    sync_clock_calibration<hires_utc_clock, cpu_counter_clock> =
        new sync_clock_calibration_type<hires_utc_clock, cpu_counter_clock>("cpu_utc");

    logger_maintenance_callback = timer::global->add_callback(100ms, [](auto current_time, auto last) {
        struct logger_maintenance_tag {
        };
        ttlet t2 = trace<logger_maintenance_tag>{};

        logger.gather_tick(last);
        logger.logger_tick();
    });

    clock_maintenance_callback = timer::global->add_callback(100ms, [](auto...) {
        struct clock_maintenance_tag {
        };
        ttlet t2 = trace<clock_maintenance_tag>{};

        sync_clock_calibration<hires_utc_clock, cpu_counter_clock>->calibrate_tick();
    });
}

void application::init_text()
{
    static_resource_view::add_static_resource(unicode_data_bin_filename, unicode_data_bin_bytes);
    static_resource_view::add_static_resource(elusiveicons_webfont_ttf_filename, elusiveicons_webfont_ttf_bytes);
    static_resource_view::add_static_resource(TTauriIcons_ttf_filename, TTauriIcons_ttf_bytes);

    unicode_data::global = std::make_unique<unicode_data>(URL("resource:unicode_data.bin"));

    font_book::global = std::make_unique<font_book>(std::vector<URL>{URL::urlFromSystemFontDirectory()});
    ElusiveIcons_font_id = font_book::global->register_font(URL("resource:elusiveicons-webfont.ttf"));
    TTauriIcons_font_id = font_book::global->register_font(URL("resource:TTauriIcons.ttf"));

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

            static_resource_view::add_static_resource(PipelineImage_vert_spv_filename, PipelineImage_vert_spv_bytes);
            static_resource_view::add_static_resource(PipelineImage_frag_spv_filename, PipelineImage_frag_spv_bytes);
            static_resource_view::add_static_resource(PipelineFlat_vert_spv_filename, PipelineFlat_vert_spv_bytes);
            static_resource_view::add_static_resource(PipelineFlat_frag_spv_filename, PipelineFlat_frag_spv_bytes);
            static_resource_view::add_static_resource(PipelineBox_vert_spv_filename, PipelineBox_vert_spv_bytes);
            static_resource_view::add_static_resource(PipelineBox_frag_spv_filename, PipelineBox_frag_spv_bytes);
            static_resource_view::add_static_resource(PipelineSDF_vert_spv_filename, PipelineSDF_vert_spv_bytes);
            static_resource_view::add_static_resource(PipelineSDF_frag_spv_filename, PipelineSDF_frag_spv_bytes);
            static_resource_view::add_static_resource(PipelineToneMapper_vert_spv_filename, PipelineToneMapper_vert_spv_bytes);
            static_resource_view::add_static_resource(PipelineToneMapper_frag_spv_filename, PipelineToneMapper_frag_spv_bytes);

            try {
                keyboardBindings.loadSystemBindings();
            } catch (std::exception const &e) {
                LOG_FATAL("Could not load keyboard bindings {}", tt::to_string(e));
            }

            gui_system::global = std::make_unique<gui_system_vulkan_win32>(gui_delegate);
            gui_system::global->init();
        }
    }
}

void application::deinit()
{
    LOG_INFO("Stopping application.");

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

    delete sync_clock_calibration<hires_utc_clock, cpu_counter_clock>;
}

void application::deinit_text()
{
    ElusiveIcons_font_id = FontID{};
    font_book::global = {};
    unicode_data::global = {};
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
