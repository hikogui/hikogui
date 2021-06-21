// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/application_delegate.hpp"
#include "ttauri/GUI/gui_system_delegate.hpp"
#include <memory>
#include <vector>

class application_controller :
    public std::enable_shared_from_this<application_controller>,
    public tt::application_delegate,
    public tt::gui_system_delegate {
public:
    static inline std::shared_ptr<application_controller> global;

    /** The delegate to be used for the gui system.
    * @return The delegate to be used for the gui system, or nullptr if the gui system should not be initialized.
    */
    virtual std::weak_ptr<tt::gui_system_delegate> gui_system_delegate(tt::application &self) noexcept {
        return weak_from_this();
    }

    std::optional<int> main(tt::application &self) override;

    void last_window_closed(tt::gui_system &self) override;
};

