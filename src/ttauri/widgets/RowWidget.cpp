
#include "RowWidget.hpp"
#include "../GUI/Window.hpp"

namespace tt {

Widget &RowWidget::addWidget(WidgetPosition position, std::unique_ptr<Widget> childWidget) noexcept
{
    auto *previous_widget = nonstd::ssize(children) != 0 ? children.back().get() : nullptr;
    if (previous_widget) {
        window.removeConstraint(rightConstraint);
    }

    auto &widget = ContainerWidget::addWidget(position, std::move(childWidget));
    if (previous_widget) {
        widget.placeRightOf(*previous_widget);
    } else {
        widget.placeLeft(0.0f);
    }
    widget.placeAtTop(0.0f);
    widget.placeAtBottom(0.0f);
    rightConstraint = widget.placeRight(0.0f);

    return widget;
}

}
