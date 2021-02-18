
#include "main_window_controller.hpp"
#include "preferences_controller.hpp"
#include "application_controller.hpp"
#include "ttauri/logger.hpp"
#include "ttauri/widgets/widgets.hpp"
#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/GUI/gui_window.hpp"

namespace demo {

void main_window_controller::init(tt::gui_window& self) noexcept
{
    using namespace tt;

    gui_window_delegate::init(self);

    // Add buttons to toolbar.
    auto preferences_button = self.make_toolbar_widget<menu_item_widget<bool>>(true);
    preferences_button->label = label{ elusive_icon::Wrench, l10n("Preferences") };
    preferences_button_callback = preferences_button->subscribe([&self]() {
        run_from_main_loop([&self]() {
            if (auto application_controller = application_controller::global.lock()) {
                self.system.make_window(
                    application_controller->preferences_controller,
                    label{ icon{URL{"resource:ttauri_demo.png"}}, l10n("TTauri Demo - Preferences") }
                );
            }
        });
    });

    auto column = self.make_widget<column_layout_widget, ""_ca>();
    auto button1 = column->make_widget<button_widget<bool>>(true);
    button1->label = l10n(u8"Hello \u4e16\u754c");
    auto button2 = column->make_widget<button_widget<bool>>(true);
    button2->label = l10n("Hello world");
    auto button3 = column->make_widget<button_widget<bool>>(true);
    button3->label = l10n("Hello earthlings");
}

}
