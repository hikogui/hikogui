// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_field_widget.hpp"
#include "selection_widget.hpp"
#include "momentary_button_widget.hpp"
#include "checkbox_widget.hpp"
#include "../audio/pcm_format.hpp"
#include <memory>

namespace tt {

class audio_device_configuration_controller {
public:

    [[nodiscard]] audio_device_configuration_controller(grid_layout_widget &grid, std::string_view address_range) noexcept;

private:
    observable<pcm_format> _pcm_selected;

    std::shared_ptr<momentary_button_widget> _device_config_button;
    std::shared_ptr<checkbox_widget> _exclusivity_checkbox;
    std::shared_ptr<text_field_widget> _num_input_channels_text_field;
    std::shared_ptr<text_field_widget> _num_output_channels_text_field;
    std::shared_ptr<selection_widget> _pcm_format_selection;
    std::shared_ptr<text_field_widget> _sample_rate_text_field;
};

}
