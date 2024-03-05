


#pragma once

#include "../dispatch/dispatch.hpp"
#include "../utility/utility.hpp"
#include "../GUI/GUI.hpp"

namespace hi { inline namespace v1 {

struct widget_delegate {
    using notifier_type = notifier<void()>;
    using callback_type = notifier_type::callback_type;

    notifier_type _notifier;

    virtual ~widget_delegate() = default;

    /** This function is called when a widget takes ownership of a delegate.
     */
    virtual void init(widget_intf const& sender) noexcept {}

    /** This function is called when a widget drops ownership of a delegate.
     */
    virtual void deinit(widget_intf const& sender) noexcept {}


    /** Subscribe a callback for notifying the widget of a data change.
     */
    template<forward_of<void()> Func>
    [[nodiscard]] callback_type subscribe(Func&& func, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return _notifier.subscribe(std::forward<Func>(func), flags);
    }
};

}}

