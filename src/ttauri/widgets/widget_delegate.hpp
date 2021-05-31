// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../notifier.hpp"
#include "../observable.hpp"
#include <type_traits>
#include <memory>

namespace tt {
class widget;

enum class widget_update_level { redraw, layout, constrain };

class widget_delegate {
public:
    using notifier_type = notifier<void(widget_update_level)>;
    using callback_ptr_type = typename notifier_type::callback_ptr_type;

    widget_delegate(widget_delegate const &) = delete;
    widget_delegate(widget_delegate &&) = delete;
    widget_delegate &operator=(widget_delegate const &) = delete;
    widget_delegate &operator=(widget_delegate &&) = delete;
    virtual ~widget_delegate() = default;

    widget_delegate() noexcept : _enabled(true), _visible(true)
    {
        _enabled_callback = _enabled.subscribe([this](auto...) {
            _notifier(widget_update_level::redraw);
        });
    }

    template<typename Func>
    [[nodiscard]] callback_ptr_type subscribe(Func &&func) noexcept
    {
        return _notifier.subscribe(std::forward<Func>(func));
    }

    virtual void init(widget &self) noexcept {}
    virtual void deinit(widget &self) noexcept {}

    virtual bool enabled(widget const &self) const noexcept
    {
        return *_enabled;
    }

    virtual void set_enabled(widget &self, observable<bool> rhs) noexcept
    {
        _enabled = std::move(rhs);
    }
    virtual bool visible(widget const &self) const noexcept
    {
        return *_visible;
    }

    virtual void set_visible(widget &self, observable<bool> rhs) noexcept
    {
        _visible = std::move(rhs);
    }

protected:
    notifier_type _notifier;

    observable<bool> _enabled;
    observable<bool> _visible;
    decltype(_enabled)::callback_ptr_type _enabled_callback;
};

} // namespace tt
