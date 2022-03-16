
#include "my_preferences_window_controller.hpp"
#include "ttauri/log.hpp"
#include "ttauri/widgets/selection_widget.hpp"
#include "ttauri/widgets/text_field_widget.hpp"
#include "ttauri/widgets/toggle_widget.hpp"
#include "ttauri/widgets/checkbox_widget.hpp"
#include "ttauri/widgets/radio_button_widget.hpp"
#include "ttauri/widgets/toolbar_tab_button_widget.hpp"
#include "ttauri/widgets/tab_widget.hpp"
#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/GUI/theme_book.hpp"
#include "ttauri/audio/audio_system.hpp"


void my_preferences_window_controller::audio_device_list_changed(tt::audio_system &system) noexcept
{
    using namespace tt;

    auto devices = system.devices();
    auto device_list = std::vector<std::pair<audio_device_id, tt::label>>{};
    for (auto const &device_ptr : devices) {
        if (device_ptr->direction() == audio_direction::output && device_ptr->state() == audio_device_state::active) {
            device_list.emplace_back(device_ptr->id, device_ptr->label());
        }
    }

    _audio_device_list = device_list;
}
