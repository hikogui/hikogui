
#include "license_preferences_controller.hpp"
#include "preferences_controller.hpp"
#include "ttauri/logger.hpp"
#include "ttauri/widgets/widgets.hpp"

namespace demo {

void license_preferences_controller::init(tt::grid_layout_widget& _self) noexcept
{
    using namespace tt;

    auto& self = dynamic_cast<grid_layout_widget&>(_self);

    auto preferences_controller_ = preferences_controller.lock();
    tt_assert(preferences_controller_);

    auto scroll = self.make_widget<vertical_scroll_view_widget<true>>("A1");
    auto grid = scroll->make_widget<>();

    grid->make_widget<label_widget>("A1", l10n("This is a toggle:"));
    auto checkbox1 = grid->make_widget<toggle_widget>("B1", preferences_controller_->toggleValue);
    checkbox1->on_label = l10n("true");
    checkbox1->off_label = l10n("false");
    checkbox1->other_label = l10n("other");

    grid->make_widget<label_widget>("A2", l10n("These is a disabled checkbox:"));
    auto checkbox2 = grid->make_widget<checkbox_widget>("B2", preferences_controller_->radioValue, 2, 0);
    checkbox2->on_label = l10n("Checkbox, with a pretty large label.");
    checkbox2->enabled = preferences_controller_->toggleValue;

    grid->make_widget<label_widget>("A3", l10n("These are radio buttons:"));
    auto radio1 = grid->make_widget<radio_button_widget>("B3", l10n("Radio 1"), preferences_controller_->radioValue, 0);
    auto radio2 = grid->make_widget<radio_button_widget>("B4", l10n("Radio 2"), preferences_controller_->radioValue, 1);
    auto radio3 = grid->make_widget<radio_button_widget>("B5", l10n("Radio 3"), preferences_controller_->radioValue, 2);

    grid->make_widget<label_widget>("A6", l10n("This is a selection box at the bottom:"));
    auto selection3 = grid->make_widget<selection_widget<int>>("B6", preferences_controller_->radioValue);
    selection3->option_list = std::vector{
        std::pair{0, label{l10n("first")}},
        std::pair{1, label{l10n("second")}},
        std::pair{2, label{l10n("third")}},
        std::pair{3, label{l10n("four")}},
        std::pair{4, label{l10n("five")}},
        std::pair{5, label{l10n("six")}},
        std::pair{6, label{l10n("seven")}}
    };
    selection3->unknown_label = l10n("Default");
    selection3->enabled = preferences_controller_->toggleValue;
}

}
