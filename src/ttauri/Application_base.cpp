// Copyright 2019 Pokitec
// All rights reserved.

#include "Application.hpp"
#include "StaticResourceView.hpp"
#include "logger.hpp"
#include "timer.hpp"
#include "os_detect.hpp"
#include "version.hpp"
#include "trace.hpp"
#include "thread.hpp"
#include "text/ElusiveIcons.hpp"
#include "text/TTauriIcons.hpp"
#include "text/language.hpp"
#include "text/FontBook.hpp"
#include "text/UnicodeData.hpp"
#include "GUI/RenderDoc.hpp"
#include "GUI/ThemeBook.hpp"
#include "GUI/KeyboardBindings.hpp"
#include "GUI/GUISystem.hpp"
#include "audio/AudioSystem.hpp"
#include <memory>

#include "data/UnicodeData.bin.inl"
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

Application_base::Application_base(
    std::shared_ptr<ApplicationDelegate> applicationDelegate,
    std::vector<std::string> const &arguments) :
    delegate(applicationDelegate)
{
    set_thread_name("Main Thread");

    // Application_base is a singleton.
    tt_assert(application == nullptr);
    application = reinterpret_cast<Application *>(this);

    tt_assert(delegate);

    application_version.name = applicationDelegate->applicationName();
    configuration = applicationDelegate->configuration(arguments);

    LOG_INFO("Starting application '{}'.", application_version.name);

    foundationStart();
    audioStart();
    textStart();
    GUIStart();
}

Application_base::~Application_base()
{
    LOG_INFO("Stopping application.");

    GUIStop();
    textStop();
    audioStop();
    foundationStop();

    // Remove the singleton.
    tt_assert(application == this);
    application = nullptr;
}

void Application_base::foundationStart()
{
    mainThreadID = std::this_thread::get_id();

    logger.minimum_log_level = static_cast<log_level>(static_cast<int>(configuration["log-level"]));

    // The logger is the first object that will use the timezone database.
    // So we will initialize it here.
#if USE_OS_TZDB == 0
    ttlet tzdata_location = URL::urlFromResourceDirectory() / "tzdata";
    date::set_install(tzdata_location.nativePath());
#endif
    try {
        timeZone = date::current_zone();
    } catch (std::runtime_error &e) {
        LOG_ERROR("Could not get the current time zone, all times shown as UTC: '{}'", e.what());
    }

    // First we need a clock, it is used by almost any other service.
    // It will immediately be synchronized, but inaccurately, it will take a while to become
    // more accurate, but we don't want to block here.
    sync_clock_calibration<hires_utc_clock,cpu_counter_clock> =
        new sync_clock_calibration_type<hires_utc_clock,cpu_counter_clock>("cpu_utc");

    logger_maintenance_cbid = maintenance_timer.add_callback(100ms, [](auto current_time, auto last) {
        struct logger_maintenance_tag {};
        ttlet t2 = trace<logger_maintenance_tag>{};

        logger.gather_tick(last);
        logger.logger_tick();
    });

    clock_maintenance_cbid = maintenance_timer.add_callback(100ms, [](auto...) {
        struct clock_maintenance_tag {};
        ttlet t2 = trace<clock_maintenance_tag>{};

        sync_clock_calibration<hires_utc_clock,cpu_counter_clock>->calibrate_tick();
    });
}

void Application_base::foundationStop()
{
    // Force all timers to finish.
    maintenance_timer.stop();
    maintenance_timer.remove_callback(clock_maintenance_cbid);
    maintenance_timer.remove_callback(logger_maintenance_cbid);

    delete sync_clock_calibration<hires_utc_clock,cpu_counter_clock>;
}

void Application_base::textStart()
{
    addStaticResource(UnicodeData_bin_filename, UnicodeData_bin_bytes);
    addStaticResource(elusiveicons_webfont_ttf_filename, elusiveicons_webfont_ttf_bytes);
    addStaticResource(TTauriIcons_ttf_filename, TTauriIcons_ttf_bytes);

    unicodeData = std::make_unique<UnicodeData>(URL("resource:UnicodeData.bin"));

    application->fonts = std::make_unique<FontBook>(std::vector<URL>{
        URL::urlFromSystemFontDirectory()
    });
    ElusiveIcons_font_id = application->fonts->register_font(URL("resource:elusiveicons-webfont.ttf"));
    TTauriIcons_font_id = application->fonts->register_font(URL("resource:TTauriIcons.ttf"));

    language::set_preferred_languages(language::get_preferred_language_tags());
    timer_preferred_languages_cbid = maintenance_timer.add_callback(1s, [](auto...){
        language::set_preferred_languages(language::get_preferred_language_tags());
    });
}

void Application_base::textStop()
{
    maintenance_timer.remove_callback(timer_preferred_languages_cbid);

    ElusiveIcons_font_id = FontID{};
    application->fonts = {};
    unicodeData = {};
}

void Application_base::audioStart()
{
}

void Application_base::audioStop()
{
    audio = {};
}

void Application_base::GUIStart()
{
    tt::guiDelegate = this;
    renderDoc = std::make_unique<RenderDoc>();

    themes = std::make_unique<ThemeBook>(std::vector<URL>{
        URL::urlFromResourceDirectory() / "themes"
    });

    themes->setThemeMode(readOSThemeMode());

    addStaticResource(PipelineImage_vert_spv_filename, PipelineImage_vert_spv_bytes);
    addStaticResource(PipelineImage_frag_spv_filename, PipelineImage_frag_spv_bytes);
    addStaticResource(PipelineFlat_vert_spv_filename, PipelineFlat_vert_spv_bytes);
    addStaticResource(PipelineFlat_frag_spv_filename, PipelineFlat_frag_spv_bytes);
    addStaticResource(PipelineBox_vert_spv_filename, PipelineBox_vert_spv_bytes);
    addStaticResource(PipelineBox_frag_spv_filename, PipelineBox_frag_spv_bytes);
    addStaticResource(PipelineSDF_vert_spv_filename, PipelineSDF_vert_spv_bytes);
    addStaticResource(PipelineSDF_frag_spv_filename, PipelineSDF_frag_spv_bytes);
    addStaticResource(PipelineToneMapper_vert_spv_filename, PipelineToneMapper_vert_spv_bytes);
    addStaticResource(PipelineToneMapper_frag_spv_filename, PipelineToneMapper_frag_spv_bytes);

    try {
        keyboardBindings.loadSystemBindings();
    } catch (error &e) {
        LOG_FATAL("Could not load keyboard bindings {}", to_string(e));
    }

    gui = std::make_unique<GUISystem>(guiDelegate);
}

void Application_base::GUIStop()
{
    gui = {};
    themes = {};
    renderDoc = {};
}


bool Application_base::initializeApplication()
{
    try {
        return delegate->initializeApplication();
    } catch (error &e) {
        LOG_FATAL("Exception during initializeApplication {}", to_string(e));
    }
}

void Application_base::audioDeviceListChanged()
{
    delegate->audioDeviceListChanged();
}

void Application_base::addStaticResource(std::string const &key, nonstd::span<std::byte const> value) noexcept
{
    staticResources.try_emplace(key, value);
}

nonstd::span<std::byte const> Application_base::getStaticResource(std::string const &key)
{
    ttlet i = staticResources.find(key);
    if (i == staticResources.end()) {
        TTAURI_THROW(key_error("Could not find static resource")
            .set<key_tag>(key)
        );
    }
    return i->second;
}

}
