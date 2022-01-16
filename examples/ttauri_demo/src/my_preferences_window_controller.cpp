
#include "my_preferences_window_controller.hpp"
#include "ttauri/log.hpp"
#include "ttauri/widgets/selection_widget.hpp"
#include "ttauri/widgets/text_field_widget.hpp"
#include "ttauri/widgets/toggle_widget.hpp"
#include "ttauri/widgets/checkbox_widget.hpp"
#include "ttauri/widgets/radio_button_widget.hpp"
#include "ttauri/widgets/toolbar_tab_button_widget.hpp"
#include "ttauri/widgets/tab_widget.hpp"
#include "ttauri/audio/audio_system.hpp"

void my_preferences_window_controller::init_audio_tab(tt::grid_widget& grid) noexcept
{
    using namespace tt;

    grid.make_widget<label_widget>("A1", l10n("Audio device:"));
    grid.make_widget<selection_widget>("B1", _audio_device_list, audio_output_device_id);

    grid.make_widget<label_widget>("A2", l10n("Sample Rate:"));
    grid.make_widget<text_field_widget>("B2", audio_output_sample_rate);
}

void my_preferences_window_controller::init_license_tab(tt::grid_widget& grid) noexcept
{
    using namespace tt;

    grid.make_widget<label_widget>("A1", l10n("This is a toggle:"));
    auto& checkbox1 = grid.make_widget<toggle_widget>("B1", toggleValue);
    checkbox1.on_label = l10n("true");
    checkbox1.off_label = l10n("false");
    checkbox1.other_label = l10n("other");

    grid.make_widget<label_widget>("A2", l10n("These is a disabled checkbox:"));
    auto& checkbox2 = grid.make_widget<checkbox_widget>("B2", radioValue, 2, 0);
    checkbox2.on_label = l10n("Checkbox, with a pretty large label.");
    checkbox2.enabled = toggleValue;

    grid.make_widget<label_widget>("A3", l10n("These are radio buttons:"));
    grid.make_widget<radio_button_widget>("B3", l10n("Radio 1"), radioValue, 0);
    grid.make_widget<radio_button_widget>("B4", l10n("Radio 2"), radioValue, 1);
    grid.make_widget<radio_button_widget>("B5", l10n("Radio 3"), radioValue, 2);

    auto option_list = std::vector{
        std::pair{0, label{l10n("first")}},
        std::pair{1, label{l10n("second")}},
        std::pair{2, label{l10n("third")}},
        std::pair{3, label{l10n("four")}},
        std::pair{4, label{l10n("five")}},
        std::pair{5, label{l10n("six")}},
        std::pair{6, label{l10n("seven")}}
    };
    grid.make_widget<label_widget>("A6", l10n("This is a selection box at the bottom:"));
    auto& selection3 = grid.make_widget<selection_widget>("B6", option_list, radioValue);
    selection3.enabled = toggleValue;
}

void my_preferences_window_controller::init(tt::gui_window& self) noexcept
{
    using namespace tt;

    gui_window_delegate::init(self);

    self.toolbar().make_widget<toolbar_tab_button_widget>(label{ elusive_icon::Speaker, l10n("Audio") }, tab_index, 0);
    self.toolbar().make_widget<toolbar_tab_button_widget>(label{ elusive_icon::Pencil, l10n("License") }, tab_index, 1);

    auto& tabs = self.content().make_widget<tab_widget>("A1", tab_index);
    init_audio_tab(tabs.make_widget<grid_widget>(0));
    init_license_tab(tabs.make_widget<scroll_widget<axis::both,true>>(1).make_widget<grid_widget>());
}

void my_preferences_window_controller::audio_device_list_changed(tt::audio_system& system) noexcept
{
    using namespace tt;

    auto devices = system.devices();
    auto device_list = std::vector<std::pair<audio_device_id, tt::label>>{};
    for (auto const& device_ptr : devices) {
        if (device_ptr->direction() == audio_direction::output && device_ptr->state() == audio_device_state::active) {
            device_list.emplace_back(device_ptr->id, device_ptr->label());
        }
    }

    _audio_device_list = device_list;
}

