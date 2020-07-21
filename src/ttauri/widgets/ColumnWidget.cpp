
#include "ColumnWidget.hpp"
#include "../GUI/Window.hpp"

namespace tt {

WidgetPosition ColumnWidget::nextPosition() noexcept
{
    return {0, -nrTopRows - 1, 1, 1};
}

}
