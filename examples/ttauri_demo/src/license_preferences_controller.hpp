// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/widgets/widget_delegate.hpp"
#include "ttauri/assert.hpp"

namespace demo {
class preferences_controller;

class license_preferences_controller : public tt::widget_delegate {
public:
    license_preferences_controller(std::weak_ptr<preferences_controller> const& preferences_controller) noexcept :
        preferences_controller(preferences_controller)
    {
        tt_assert(!preferences_controller.expired());
    }

    void init(tt::widget& self) noexcept override;

protected:
    std::weak_ptr<preferences_controller> preferences_controller;
};

}
