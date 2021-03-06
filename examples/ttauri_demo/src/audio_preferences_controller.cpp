
#include "audio_preferences_controller.hpp"
#include "preferences_controller.hpp"
#include "application_preferences.hpp"
#include "ttauri/logger.hpp"
#include "ttauri/widgets/widgets.hpp"

namespace demo {

void audio_preferences_controller::init(tt::grid_layout_widget& self) noexcept
{
    using namespace tt;

    auto preferences_controller_ = preferences_controller.lock();
    tt_assert(preferences_controller_);

    self.make_widget<label_widget>("A1", l10n("Audio device sample rate:"));
    self.make_widget<text_field_widget<int>>("B1", preferences_controller_->radioValue);

    self.make_widget<label_widget>("A2", l10n("Word clock sample rate:"));
    self.make_widget<text_field_widget<int>>("B2", preferences_controller_->radioValue);

    self.make_widget<label_widget>("A3", l10n("This is a toggle:"));
    auto toggle1 = self.make_widget<toggle_widget>("B3", preferences_controller_->toggleValue);
    toggle1->on_label = l10n("on");
    toggle1->off_label = l10n("off");

    self.make_widget<label_widget>("A4", l10n("This is disabled toggle:"));
    auto toggle2 = self.make_widget<toggle_widget>("B4", !preferences_controller_->toggleValue);
    toggle2->on_label = l10n("on");
    toggle2->enabled = false;

    self.make_widget<label_widget>("A5", l10n("Audio device:"));
    auto audio_device_selector = self.make_widget<selection_widget<std::string>>("B5");
    audio_device_selector->value = application_preferences::global->audio_output_device_id;
    audio_device_selector->option_list = preferences_controller_->_audio_device_list;
    audio_device_selector->unknown_label = l10n("No device selected.");

    self.make_widget<label_widget>("A6", l10n("This is disabled selection widget:"));
    auto selection2 = self.make_widget<selection_widget<int>>("B6",
        preferences_controller_->radioValue,
        std::vector{
            std::pair{0, label{l10n("first")}},
            std::pair{1, label{l10n("The second value has a really long label")}},
            std::pair{2, label{l10n("third")}}
        },
        l10n("Default"));
    selection2->enabled = false;

    self.make_widget<label_widget>("A7", l10n("These are radio buttons:"));
    self.make_widget<radio_button_widget<int>>("B7", 0, preferences_controller_->radioValue, label(l10n("Radio 1.")));

    auto radio2 = self.make_widget<radio_button_widget<int>>("B8", 1, preferences_controller_->radioValue);
    radio2->label = l10n("Radio 2, with another large label.");

    auto radio3 = self.make_widget<radio_button_widget<int>>("B9", 2);
    radio3->label = l10n("Radio 3");
    radio3->value = preferences_controller_->radioValue;
    radio3->enabled = false;
}

}
