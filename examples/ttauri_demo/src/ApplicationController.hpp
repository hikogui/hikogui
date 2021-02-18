// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "MainWindowController.hpp"
#include "PreferencesController.hpp"
#include "ttauri/application_delegate.hpp"
#include "ttauri/audio/audio_system_delegate.hpp"
#include "ttauri/GUI/gui_system_delegate.hpp"
#include <memory>
#include <vector>

using namespace tt;

namespace ttauri_demo {

class ApplicationController : public std::enable_shared_from_this<ApplicationController>, public tt::application_delegate, public tt::audio_system_delegate, public tt::gui_system_delegate {
public:
    std::shared_ptr<MainWindowController> main_window_controller;
    std::shared_ptr<PreferencesController> preferences_controller;

    static inline std::weak_ptr<ApplicationController> global;

    ApplicationController() noexcept {
        main_window_controller = std::make_shared<MainWindowController>();
        preferences_controller = std::make_shared<PreferencesController>();
    }

    std::string application_name(application &self) const noexcept override;

    tt::datum configuration(application &self, std::vector<std::string> arguments) const noexcept override;

    /** The delegate to be used for the audio system.
    * @return The delegate to be used for the audio system, or nullptr if the audio system should not be initialized.
    */
    virtual std::weak_ptr<tt::audio_system_delegate> audio_system_delegate(application &self) noexcept override {
        return weak_from_this();
    }

    /** The delegate to be used for the gui system.
    * @return The delegate to be used for the gui system, or nullptr if the gui system should not be initialized.
    */
    virtual std::weak_ptr<tt::gui_system_delegate> gui_system_delegate(application &self) noexcept {
        return weak_from_this();
    }

    std::optional<int> main(tt::application &self) override;

    void last_window_closed(tt::gui_system &self) override;

    void audio_device_list_changed(tt::audio_system &self) override;
};

}
