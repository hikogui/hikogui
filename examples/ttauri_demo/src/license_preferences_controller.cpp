
#include "license_preferences_controller.hpp"
#include "preferences_controller.hpp"
#include "ttauri/logger.hpp"
#include "ttauri/widgets/widgets.hpp"

namespace demo {

void license_preferences_controller::init(tt::grid_layout_widget &self) noexcept
{
    using namespace tt;

    auto preferences_controller_ = preferences_controller.lock();
    tt_assert(preferences_controller_);

    auto scroll = self.make_widget<vertical_scroll_view_widget<true>,""_ca>();
    auto grid = scroll->make_widget<>();

    grid->make_widget<label_widget,"L0T+1"_ca>(l10n("These is a checkbox:"));
    auto checkbox1 = grid->make_widget<checkbox_widget<int>,"L1"_ca>(0, 2);
    checkbox1->true_label = l10n("true");
    checkbox1->false_label = l10n("false");
    checkbox1->other_label = l10n("other");
    checkbox1->value = preferences_controller_->radioValue;

    grid->make_widget<label_widget,"L0T+1"_ca>(l10n("These is a disabled checkbox:"));
    auto checkbox2 = grid->make_widget<checkbox_widget<int>,"L1"_ca>(0, 2);
    checkbox2->true_label = l10n("Checkbox, with a pretty large label.");
    checkbox2->enabled = false;
    checkbox2->value = preferences_controller_->radioValue;

    grid->make_widget<label_widget,"L0T+1"_ca>(l10n("This is a selection box at the bottom:"));
    auto selection3 = grid->make_widget<selection_widget<int>,"L1"_ca>(preferences_controller_->radioValue);
    selection3->option_list = std::vector{
        std::pair{0, label{l10n("first")}},
        std::pair{1, label{l10n("second")}},
        std::pair{2, label{l10n("third")}}
    };
    selection3->unknown_label = l10n("Default");
    selection3->enabled = true;
}

}
