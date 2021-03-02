// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "main_window_controller.hpp"
#include "preferences_controller.hpp"
#include "ttauri/application_delegate.hpp"
#include "ttauri/audio/audio_system_delegate.hpp"
#include "ttauri/GUI/gui_system_delegate.hpp"
#include <memory>
#include <vector>

namespace demo {

class application_controller : public std::enable_shared_from_this<application_controller>, public tt::application_delegate, public tt::audio_system_delegate, public tt::gui_system_delegate {
public:
    std::shared_ptr<main_window_controller> main_window_controller;
    std::shared_ptr<preferences_controller> preferences_controller;

    static inline std::weak_ptr<application_controller> global;

    application_controller() noexcept {
        main_window_controller = std::make_shared<demo::main_window_controller>();
        preferences_controller = std::make_shared<demo::preferences_controller>();
    }

    tt::version application_version(tt::application &self) const noexcept override;

    tt::datum configuration(tt::application &self, std::vector<std::string> arguments) const noexcept override;

    /** The delegate to be used for the audio system.
    * @return The delegate to be used for the audio system, or nullptr if the audio system should not be initialized.
    */
    virtual std::weak_ptr<tt::audio_system_delegate> audio_system_delegate(tt::application &self) noexcept override {
        return weak_from_this();
    }

    /** The delegate to be used for the gui system.
    * @return The delegate to be used for the gui system, or nullptr if the gui system should not be initialized.
    */
    virtual std::weak_ptr<tt::gui_system_delegate> gui_system_delegate(tt::application &self) noexcept {
        return weak_from_this();
    }

    std::optional<int> main(tt::application &self) override;

    void last_window_closed(tt::gui_system &self) override;

    void audio_device_list_changed(tt::audio_system &self) override;
};

}
