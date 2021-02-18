// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/widgets/grid_layout_delegate.hpp"
#include "ttauri/assert.hpp"

namespace ttauri_demo {
class PreferencesController;

class LicensePreferencesController : public tt::grid_layout_delegate {
public:
    LicensePreferencesController(std::weak_ptr<PreferencesController> const& preferences_controller) noexcept :
        preferences_controller(preferences_controller)
    {
        tt_assert(!preferences_controller.expired());
    }

    void init(tt::grid_layout_widget& self) noexcept override;

protected:
    std::weak_ptr<PreferencesController> preferences_controller;
};

}