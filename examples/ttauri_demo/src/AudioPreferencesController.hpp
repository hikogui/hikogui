// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/label.hpp"
#include "ttauri/observable.hpp"
#include "ttauri/widgets/grid_layout_delegate.hpp"

namespace ttauri_demo {
class PreferencesController;

class AudioPreferencesController : public tt::grid_layout_delegate {
public:
    AudioPreferencesController(std::weak_ptr<PreferencesController> const &preferences_controller) noexcept :
        preferences_controller(preferences_controller)
    {
        tt_assert(!preferences_controller.expired());
    }

    void init(tt::grid_layout_widget &self) noexcept override;

protected:
    std::weak_ptr<PreferencesController> preferences_controller;
};

}