
#include "ColumnWidget.hpp"
#include "ttauri/GUI/Window.hpp"

namespace tt {

Widget &ColumnWidget::addWidget(Alignment alignment, std::unique_ptr<Widget> childWidget) noexcept
{
    auto *previous_widget = ssize(children) != 0 ? children.back().get() : nullptr;
    if (previous_widget) {
        window.removeConstraint(bottomConstraint);
    }

    auto &widget = Widget::addWidget(alignment, std::move(childWidget));
    if (previous_widget) {
        widget.placeBelow(*previous_widget);
    } else {
        widget.placeAtTop(0.0f);
    }
    widget.placeLeft(0.0f);
    widget.placeRight(0.0f);
    bottomConstraint = widget.placeAtBottom(0.0f);

    return widget;
}

}
