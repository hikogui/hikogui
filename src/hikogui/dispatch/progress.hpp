// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "notifier.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <memory>
#include <mutex>
#include <chrono>

hi_export_module(hikogui.dispatch.progress);

hi_export namespace hi {
inline namespace v1 {

class progress_sink;

class progress_token {
public:
    constexpr progress_token() noexcept = default;

    void set_value(float value);

    progress_token& operator=(float value)
    {
        set_value(value);
        return *this;
    }

private:
    progress_sink* _sink = nullptr;

    constexpr progress_token(progress_sink* sink) noexcept : _sink(sink) {}

    friend class progress_sink;
};

class progress_sink {
public:
    using callback_type = notifier<>::callback_type;
    
    constexpr progress_sink() noexcept = default;

    [[nodiscard]] progress_token get_token() const noexcept
    {
        return progress_token{const_cast<progress_sink*>(this)};
    }

    void reset()
    {
        set_value(0.0f);
    }

    void set_value(float value)
    {
        _value = value;
        _notifier();
    }

    [[nodiscard]] constexpr float value() const noexcept
    {
        return _value;
    }

    [[nodiscard]] float operator*() const noexcept
    {
        return _value;
    }

    template<typename Callback>
    [[nodiscard]] notifier<>::callback_type subscribe(Callback&& callback, callback_flags flags = callback_flags::synchronous)
    {
        return _notifier.subscribe(std::forward<Callback>(callback), flags);
    }

private:
    notifier<> _notifier = {};
    float _value = 0.0f;
};

inline void progress_token::set_value(float value)
{
    hi_axiom_bounds(value, 0.0f, 1.0f);
    if (_sink != nullptr) {
        _sink->set_value(value);
    }
}

} // namespace v1
}
