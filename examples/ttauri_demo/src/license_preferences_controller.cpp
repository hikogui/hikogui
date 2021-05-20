
#include "license_preferences_controller.hpp"
#include "preferences_controller.hpp"
#include "ttauri/logger.hpp"
#include "ttauri/widgets/widgets.hpp"

namespace demo {

void license_preferences_controller::init(tt::widget &_self) noexcept
{
    using namespace tt;

    auto &self = dynamic_cast<grid_layout_widget&>(_self);

    auto preferences_controller_ = preferences_controller.lock();
    tt_assert(preferences_controller_);

    auto scroll = self.make_widget<vertical_scroll_view_widget<true>>("A1");
    auto grid = scroll->make_widget<>();

    grid->make_widget<label_widget>("A1", l10n("These is a checkbox:"));
    auto checkbox1 = grid->make_widget<boolean_checkbox_widget>("B1");
    checkbox1->set_on_label(l10n("true"));
    checkbox1->set_off_label(l10n("false"));
    checkbox1->set_other_label(l10n("other"));
    checkbox1->set_value(preferences_controller_->toggleValue);

    grid->make_widget<label_widget>("A2", l10n("These is a disabled checkbox:"));
    auto checkbox2 = grid->make_widget<checkbox_widget<int>>("B2");
    checkbox2->set_on_value(2);
    checkbox2->set_off_value(0);
    checkbox2->set_on_label(l10n("Checkbox, with a pretty large label."));
    checkbox2->set_enabled(preferences_controller_->toggleValue);
    checkbox2->set_value(preferences_controller_->radioValue);

    grid->make_widget<label_widget>("A3", l10n("These are radio buttons:"));
    auto radio1 = grid->make_widget<radio_button_widget<int>>("B3", preferences_controller_->radioValue);
    radio1->set_on_value(0);
    radio1->set_label(l10n("Radio 1"));
    auto radio2 = grid->make_widget<radio_button_widget<int>>("B4", preferences_controller_->radioValue);
    radio2->set_on_value(1);
    radio2->set_label(l10n("Radio 2"));
    auto radio3 = grid->make_widget<radio_button_widget<int>>("B5", preferences_controller_->radioValue);
    radio3->set_on_value(2);
    radio3->set_label(l10n("Radio 3"));

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
    selection3->set_enabled(preferences_controller_->toggleValue);
}

}
