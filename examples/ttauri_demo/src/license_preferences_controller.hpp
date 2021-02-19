// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/widgets/grid_layout_delegate.hpp"
#include "ttauri/assert.hpp"

namespace demo {
class preferences_controller;

class license_preferences_controller : public tt::grid_layout_delegate {
public:
    license_preferences_controller(std::weak_ptr<preferences_controller> const& preferences_controller) noexcept :
        preferences_controller(preferences_controller)
    {
        tt_assert(!preferences_controller.expired());
    }

    void init(tt::grid_layout_widget& self) noexcept override;

protected:
    std::weak_ptr<preferences_controller> preferences_controller;
};

}
