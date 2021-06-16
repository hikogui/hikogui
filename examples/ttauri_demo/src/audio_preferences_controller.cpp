
#include "audio_preferences_controller.hpp"
#include "preferences_controller.hpp"
#include "application_preferences.hpp"
#include "ttauri/logger.hpp"
#include "ttauri/widgets/widgets.hpp"

namespace demo {

void audio_preferences_controller::init(tt::grid_layout_widget& _self) noexcept
{
    using namespace tt;

    auto& self = dynamic_cast<tt::grid_layout_widget&>(_self);

    auto preferences_controller_ = preferences_controller.lock();
    tt_assert(preferences_controller_);

    self.make_widget<label_widget>("A1", l10n("Audio device:"));
    auto audio_device_selector = self.make_widget<selection_widget>(
        "B1",
        l10n("No device selected."),
        preferences_controller_->_audio_device_list,
        application_preferences::global->audio_output_device_id);

    _audio_device_configurator = std::make_shared<audio_device_configuration_controller>(self, "A2:B7");

    self.make_widget<label_widget>("A8", l10n("Word clock sample rate:"));
    self.make_widget<text_field_widget>("B8", preferences_controller_->radioValue);

}

}
