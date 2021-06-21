// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "datum.hpp"
#include <string>
#include <vector>
#include <optional>

namespace tt {
class gfx_system;
class gfx_system_delegate;
class application;

/** application Delegate.
 * Can be subclasses by the actual application to be called when certain events happen.
 */
class application_delegate {
public:
    application_delegate() = default;
    virtual ~application_delegate() = default;

    virtual void init(application &self) {}
    virtual void deinit(application &self) {}

    /** The delegate to be used for the gui system.
     * @return The delegate to be used for the gui system, or nullptr if the gui system should not be initialized.
     */
    virtual std::weak_ptr<gfx_system_delegate> gfx_system_delegate(application &self) noexcept
    {
        return {};
    }

    /** Initialize the application.
     * Called right before the application loop is started.
     *
     * @return An exit value, or empty to run the main-loop.
     */
    virtual std::optional<int> main(application &self) = 0;

};

}
