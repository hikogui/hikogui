
#include "preferences_controller.hpp"
#include "ttauri/logger.hpp"
#include "ttauri/widgets/widgets.hpp"
#include "ttauri/audio/audio_system.hpp"

namespace demo {

void preferences_controller::init(tt::gui_window &self) noexcept
{
    using namespace tt;

    gui_window_delegate::init(self);

    audio_preferences_controller = std::make_shared<demo::audio_preferences_controller>(weak_from_this());
    license_preferences_controller = std::make_shared<demo::license_preferences_controller>(weak_from_this());

    auto tab0 = self.make_toolbar_widget<toolbar_tab_button_widget<int>>(0, tab_index);
    tab0->label = {elusive_icon::Speaker , l10n("Audio")};

    auto tab1 = self.make_toolbar_widget<toolbar_tab_button_widget<int>>(1, tab_index);
    tab1->label = {elusive_icon::Pencil, l10n("License")};

    auto tabs = self.make_widget<tab_view_widget<int>,"L0T0"_ca>(tab_index);
    tabs->make_widget(0, audio_preferences_controller);
    tabs->make_widget(1, license_preferences_controller);
}

void preferences_controller::audio_device_list_changed(tt::audio_system &system) noexcept
{
    using namespace tt;

    auto devices = system.devices();
    auto device_list = std::vector<std::pair<std::string,tt::label>>{};
    for (auto const &device_ptr : devices) {
        if (device_ptr->direction() == audio_device_flow_direction::output && device_ptr->state() == audio_device_state::active) {
            device_list.emplace_back(device_ptr->id(), device_ptr->label());
        }
    }

    _audio_device_list = device_list;
}

}
