// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <optional>

namespace hi::inline v1 {
class gui_system;

class gui_system_delegate {
public:
    [[nodiscard]] virtual void init(gui_system &self) noexcept {};

    [[nodiscard]] virtual void deinit(gui_system &self) noexcept {};

    /** This function is called when the last window is closed.
     * @param An exit code if the gui-system's event-loop should exit; otherwise empty.
     */
    [[nodiscard]] virtual std::optional<int> last_window_closed(gui_system &self)
    {
        return 0;
    };
};

} // namespace hi::inline v1
