// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "Application_forward.hpp"
#include "GUI/gui_system_delegate.hpp"
#include "audio/audio_system_delegate.hpp"
#include "datum.hpp"
#include <string>
#include <vector>

namespace tt {

/** Application Delegate.
 * Can be subclasses by the actual application to be called when certain events happen.
 */
class application_delegate {
public:
    application_delegate() = default;
    virtual ~application_delegate() = default;

    /*! Called when an application name is needed.
     */
    virtual std::string application_name() const noexcept = 0;

    /*! Return the possible command line argument options.
     */
    virtual datum configuration(std::vector<std::string> arguments) const noexcept = 0;

    /** The delegate to be used for the audio system.
     * @return The delegate to be used for the audio system, or nullptr if the audio system should not be initialized.
     */
    virtual tt::audio_system_delegate *audio_system_delegate() noexcept { return nullptr; }

    /** The delegate to be used for the gui system.
     * @return The delegate to be used for the gui system, or nullptr if the gui system should not be initialized.
     */
    virtual gui_system_delegate *gui_system_delegate() noexcept { return nullptr; }

    /** Initialize the application.
     * Called right before the application loop is started.
     *
     * @return true to start the loop, false to exit the application.
     */
    virtual bool initialize_application() = 0;
};

}
