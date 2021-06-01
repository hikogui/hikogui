// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_device_configuration_controller.hpp"
#include "grid_layout_widget.hpp"
#include "label_widget.hpp"

namespace tt {

audio_device_configuration_controller::audio_device_configuration_controller(
    grid_layout_widget &grid,
    std::string_view address_range) noexcept
{
    auto [column_nr, row_nr, column_nr2, row_nr2] = parse_spread_sheet_range(address_range);

    if (column_nr2 - column_nr < 2) {
        tt_log_fatal("audio_device_configuration_controller requires two columns on the grid, given {}", address_range);
    }

    if (row_nr2 - row_nr < 5) {
        tt_log_fatal("audio_device_configuration_controller requires five rows on the grid, given {}", address_range);
    }

    _device_config_button = grid.make_widget<label_button_widget<bool>>(column_nr + 1, row_nr);
    _device_config_button->set_label(l10n("Sound Control Panel"));
    ++row_nr;

    grid.make_widget<label_widget>(column_nr, row_nr, l10n("Exclusive mode:"));
    _exclusivity_checkbox = grid.make_widget<checkbox_widget<bool>>(column_nr + 1, row_nr);
    ++row_nr;

    grid.make_widget<label_widget>(column_nr, row_nr, l10n("Number of input channels:"));
    _num_input_channels_text_field = grid.make_widget<text_field_widget<int>>(column_nr + 1, row_nr);
    ++row_nr;

    grid.make_widget<label_widget>(column_nr, row_nr, l10n("Number of output channels:"));
    _num_output_channels_text_field = grid.make_widget<text_field_widget<int>>(column_nr + 1, row_nr);
    ++row_nr;

    grid.make_widget<label_widget>(column_nr, row_nr, l10n("Sample format:"));
    _pcm_format_selection = grid.make_widget<selection_widget<pcm_format>>(column_nr + 1, row_nr);
    _pcm_format_selection->option_list = std::vector{
        std::pair{pcm_format::int16, label{l10n("16 bit integer PCM")}},
        std::pair{pcm_format::int20, label{l10n("20 bit integer PCM")}},
        std::pair{pcm_format::int24, label{l10n("24 bit integer PCM")}}};
    ++row_nr;
    
    grid.make_widget<label_widget>(column_nr, row_nr, l10n("Audio device sample rate:"));
    _sample_rate_text_field = grid.make_widget<text_field_widget<int>>(column_nr + 1, row_nr);
}

} // namespace tt
