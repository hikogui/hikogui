// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget_delegate.hpp"
#include "../label.hpp"

namespace tt {

class label_delegate : public widget_delegate {
public:
    label_delegate() noexcept : _label(l10n("<unknown label>"))
    {
        _label_callback = _label.subscribe([this](auto...) {
            _notifier(widget_update_level::constrain);
        });
    }

    virtual label label(widget const &sender) const noexcept
    {
        return *_label;
    }

    virtual void set_label(widget &sender, observable<tt::label> rhs) noexcept
    {
        _label = std::move(rhs);
    }

    std::string text(widget const &sender) const noexcept
    {
        return (*_label).text();
    }

    template<typename... Args>
    void set_text(l10n fmt, Args &&... args) noexcept
    {
        auto label = *_label;
        label.set_text(fmt, std::forward<Args>(args)...);
        _label = label;
    }

    tt::icon const &icon(widget const &sender) const noexcept
    {
        return (*_label).icon();
    }

    void set_icon(widget &sender, tt::icon const &icon) noexcept
    {
        auto label = *_label;
        label.set_icon(icon);
        _label = label;
    }

protected:
    observable<tt::label> _label;
    decltype(_label)::callback_ptr_type _label_callback;
};


}
