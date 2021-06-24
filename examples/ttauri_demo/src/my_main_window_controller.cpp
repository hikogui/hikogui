
#include "my_main_window_controller.hpp"
#include "my_preferences_window_controller.hpp"
#include "ttauri/logger.hpp"
#include "ttauri/widgets/widgets.hpp"
#include "ttauri/GUI/gui_system.hpp"

void my_main_window_controller::init(tt::gui_window& window) noexcept
{
    using namespace tt;

    super::init(window);

    // Add buttons to toolbar.
    auto &preferences_button = window.make_toolbar_widget<toolbar_button_widget>(label{ elusive_icon::Wrench, l10n("Preferences") });
    preferences_button_callback = preferences_button.subscribe([&window]() {
        gui_system::global().make_window(
            my_preferences_window_controller::global,
            label{ icon{URL{"resource:ttauri_demo.png"}}, l10n("TTauri Demo - Preferences") }
        );
    });

    auto &column = window.make_widget<column_layout_widget>("A1");
    column.make_widget<momentary_button_widget>(l10n("Hello \u4e16\u754c"));
    column.make_widget<momentary_button_widget>(l10n("Hello world"));
    column.make_widget<momentary_button_widget>(l10n("Hello earthlings"));
}
