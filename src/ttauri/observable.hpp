// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "notifier.hpp"
#include "hires_utc_clock.hpp"
#include "cast.hpp"
#include "notifier.hpp"
#include "detail/observable_value.hpp"
#include "detail/observable_not.hpp"
#include <memory>
#include <functional>
#include <algorithm>

namespace tt {

template<typename T>
class observable {
public:
    using value_type = T;
    using notifier_type = notifier<void()>;
    using callback_type = typename notifier_type::callback_type;
    using callback_ptr_type = typename notifier_type::callback_ptr_type;
    using time_point = hires_utc_clock::time_point;
    using duration = hires_utc_clock::duration;

    observable(observable const &other) noexcept : pimpl(other.pimpl)
    {
        tt_axiom(&other != this);
        pimpl_callback = pimpl->subscribe([this]() {
            this->notifier();
        });
    }

    observable &operator=(observable const &other) noexcept
    {
        // Self-assignment is allowed.
        pimpl->unsubscribe(pimpl_callback);
        pimpl = other.pimpl;
        pimpl_callback = pimpl->subscribe([this]() {
            this->notifier();
        });
        this->notifier();
        return *this;
    }

    // Use the copy constructor and assignment operator.
    // observable(observable &&other) noexcept = delete;
    // observable &operator=(observable &&other) noexcept = delete;

    ~observable()
    {
        tt_axiom(pimpl);
    }

    observable() noexcept :
        observable(std::static_pointer_cast<detail::observable_base<value_type>>(
            std::make_shared<detail::observable_value<value_type>>()))
    {
    }

    observable(value_type const &value) noexcept :
        observable(std::static_pointer_cast<detail::observable_base<value_type>>(
            std::make_shared<detail::observable_value<value_type>>(value)))
    {
    }

    observable &operator=(value_type const &value) noexcept
    {
        store(value);
        return *this;
    }

    observable &operator+=(value_type const &value) noexcept
    {
        store(load() + value);
        return *this;
    }

    [[nodiscard]] value_type previous_value() const noexcept
    {
        tt_axiom(pimpl);
        return pimpl->previous_value();
    }

    /** Time when the value was modified last.
     */
    [[nodiscard]] time_point time_when_last_modified() const noexcept
    {
        tt_axiom(pimpl);
        return pimpl->time_when_last_modified();
    }

    /** Duration since the value was last modified.
     */
    [[nodiscard]] duration duration_since_last_modified() const noexcept
    {
        tt_axiom(pimpl);
        return pimpl->duration_since_last_modified();
    }

    /** The relative time since the start of the animation.
     * The relative time since the start of the animation according to the duration of the animation.
     *
     * @param animation_duration The duration of the full animation.
     * @return The relative animation progress between 0.0 and 1.0.
     */
    [[nodiscard]] float animation_progress(duration animation_duration) const noexcept
    {
        tt_axiom(pimpl);
        return pimpl->animation_progress(animation_duration);
    }

    [[nodiscard]] bool animating(duration animation_duration) const noexcept
    {
        tt_axiom(pimpl);
        return pimpl->animation_progress(animation_duration) < 1.0f;
    }

    [[nodiscard]] value_type load() const noexcept
    {
        tt_axiom(pimpl);
        return pimpl->load();
    }

    [[nodiscard]] value_type operator*() const noexcept
    {
        tt_axiom(pimpl);
        return pimpl->load();
    }

    [[nodiscard]] value_type load(duration animation_duration) const noexcept
    {
        tt_axiom(pimpl);
        return pimpl->load(animation_duration);
    }

    bool store(value_type const &new_value) noexcept
    {
        tt_axiom(pimpl);
        return pimpl->store(new_value);
    }

    template<typename Callback>
    [[nodiscard]] callback_ptr_type subscribe(Callback &&callback) noexcept
    {
        return notifier.subscribe(std::forward<Callback>(callback));
    }

    void subscribe_ptr(callback_ptr_type const &callback) noexcept
    {
        return notifier.subscribe_ptr(callback);
    }

    void unsubscribe(callback_ptr_type const &callback_ptr) noexcept
    {
        return notifier.unsubscribe(callback_ptr);
    }

    [[nodiscard]] friend observable<bool> operator!(observable const &rhs) noexcept
    {
        return std::static_pointer_cast<detail::observable_base<bool>>(std::make_shared<detail::observable_not<bool>>(rhs.pimpl));
    }

    [[nodiscard]] friend bool operator==(observable const &lhs, observable const &rhs) noexcept
    {
        return *lhs == *rhs;
    }

    [[nodiscard]] friend bool operator==(observable const &lhs, value_type const &rhs) noexcept
    {
        return *lhs == rhs;
    }

    [[nodiscard]] friend bool operator==(value_type const &lhs, observable const &rhs) noexcept
    {
        return lhs == *rhs;
    }

    [[nodiscard]] friend bool operator!=(observable const &lhs, observable const &rhs) noexcept
    {
        return *lhs != *rhs;
    }

    [[nodiscard]] friend bool operator!=(observable const &lhs, value_type const &rhs) noexcept
    {
        return *lhs != rhs;
    }

    [[nodiscard]] friend bool operator!=(value_type const &lhs, observable const &rhs) noexcept
    {
        return lhs != *rhs;
    }

    [[nodiscard]] friend float to_float(observable const &rhs) noexcept
    {
        return narrow_cast<float>(rhs.load());
    }

    [[nodiscard]] friend float to_float(observable const &rhs, duration animation_duration) noexcept
    {
        ttlet previous_value = narrow_cast<float>(rhs.previous_value());
        ttlet current_value = narrow_cast<float>(rhs.load());
        ttlet animation_progress = rhs.animation_progress(animation_duration);
        return mix(animation_progress, previous_value, current_value);
    }

    [[nodiscard]] friend std::string to_string(observable const &rhs) noexcept
    {
        return to_string(rhs.load());
    }

    friend std::ostream &operator<<(std::ostream &lhs, observable const &rhs) noexcept
    {
        return lhs << rhs.load();
    }

private:
    using pimpl_type = detail::observable_base<value_type>;

    notifier_type notifier;
    std::shared_ptr<pimpl_type> pimpl;
    typename pimpl_type::callback_ptr_type pimpl_callback;

    observable(std::shared_ptr<detail::observable_base<value_type>> const &other) noexcept : pimpl(other)
    {
        pimpl_callback = pimpl->subscribe([this]() {
            this->notifier();
        });
    }

    observable(std::shared_ptr<detail::observable_base<value_type>> &&other) noexcept : pimpl(std::move(other))
    {
        pimpl_callback = pimpl->subscribe([this]() {
            this->notifier();
        });
    }

    observable &operator=(std::shared_ptr<detail::observable_base<value_type>> const &other) noexcept
    {
        pimpl->unsubscribe(pimpl_callback);
        pimpl = other;
        pimpl_callback = pimpl->subscribe([this]() {
            this->notifier();
        });

        this->notifier(this->load());
        return *this;
    }
};

} // namespace tt
