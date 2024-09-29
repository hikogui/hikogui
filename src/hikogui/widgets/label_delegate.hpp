

#pragma once

#include "text_delegate.hpp"
#include "icon_delegate.hpp"

namespace hi::inline v1 {

class label_delegate : public virtual text_delegate, public virtual icon_delegate {
};

template<typename... Args>
class default_label_delegate;

template<>
class default_label_delegate<> : public label_delegate {

template<>
class default_label_delegate<hi::label> : public label_delegate {
public:
    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     */
    template<forward_of<observer<hi::label>> Value>
    explicit default_label_delegate(Value&& value) : _value(std::forward<Value>(value))
    {
        _value_cbt = this->_value.subscribe([&](auto...) {
            this->_notifier();
        });
    }

    [[nodiscard]] bool empty_text(widget_intf const* sender) const override
    {
        return value->text.empty();
    }

    [[nodiscard]] bool empty_icon(widget_intf const* sender) const override
    {
        return value->icon.empty();
    }

    [[nodiscard]] gstring get_text(widget_intf const* sender) const override
    {
        return to_gstring(value->text.translate());
    }

    [[nodiscard]] hi::icon get_icon(widget_intf const* sender) const override
    {
        return value->icon;
    }

private:
    observer<hi::label> _value;
    callback<void(hi::label)> _value_cbt;
};

class default_label_delegate<hi::icon> : public label_delegate {
public:
    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     */
    template<forward_of<observer<hi::icon>> Value>
    explicit default_label_delegate(Value&& value) : _value(std::forward<Value>(value))
    {
        _value_cbt = this->_value.subscribe([&](auto...) {
            this->_notifier();
        });
    }

    [[nodiscard]] bool empty_text(widget_intf const* sender) const override
    {
        return true;
    }

    [[nodiscard]] bool empty_icon(widget_intf const* sender) const override
    {
        return value->icon.empty();
    }

    [[nodiscard]] gstring get_text(widget_intf const* sender) const override
    {
        return {};
    }

    [[nodiscard]] hi::icon get_icon(widget_intf const* sender) const override
    {
        return value->icon;
    }

private:
    observer<hi::icon> _value;
    callback<void(hi::icon)> _value_cbt;
};

class default_label_delegate<hi::txt> : public label_delegate {
public:
    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     */
    template<forward_of<observer<hi::txt>> Value>
    explicit default_label_delegate(Value&& value) : _value(std::forward<Value>(value))
    {
        _value_cbt = this->_value.subscribe([&](auto...) {
            this->_notifier();
        });
    }

    [[nodiscard]] bool empty_text(widget_intf const* sender) const override
    {
        return value->text.empty();
    }

    [[nodiscard]] bool empty_icon(widget_intf const* sender) const override
    {
        return true
    }

    [[nodiscard]] gstring get_text(widget_intf const* sender) const override
    {
        return to_gstring(value->text.translate());
    }

    [[nodiscard]] hi::icon get_icon(widget_intf const* sender) const override
    {
        return {};
    }

private:
    observer<hi::txt> _value;
    callback<void(hi::txt)> _value_cbt;
};


template<forward_of<observer<hi::txt>> Value>
default_text_delegate(Value&&) -> default_text_delegate<observer_decay_t<Value>>;

template<forward_of<observer<hi::icon>> Value>
default_text_delegate(Value&&) -> default_text_delegate<observer_decay_t<Value>>;

template<forward_of<observer<hi::label>> Value>
default_text_delegate(Value&&) -> default_text_delegate<observer_decay_t<Value>>;

}
