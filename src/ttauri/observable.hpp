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
#include "algorithm.hpp"
#include <memory>
#include <functional>
#include <algorithm>
#include <type_traits>

namespace tt {

template<typename T>
class observable {
public:
    using value_type = T;
    using notifier_type = notifier<void()>;
    using callback_type = typename notifier_type::callback_type;
    using callback_ptr_type = typename notifier_type::callback_ptr_type;

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

    /** Is the internal value true.
     */
    explicit operator bool () const noexcept
    {
        return static_cast<bool>(load());
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

    bool store(value_type const &new_value) noexcept
    {
        tt_axiom(pimpl);
        return pimpl->store(new_value);
    }

    template<typename Callback> requires(std::is_invocable_v<Callback>)
    [[nodiscard]] callback_ptr_type subscribe(Callback &&callback) noexcept
    {
        return notifier.subscribe(std::forward<Callback>(callback));
    }

    callback_ptr_type subscribe(callback_ptr_type const &callback) noexcept
    {
        return notifier.subscribe(callback);
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

template<typename T>
struct is_observable : public std::false_type {};

template<typename V>
struct is_observable<observable<V>> : public std::true_type {};

template<typename T>
constexpr bool is_observable_v = is_observable<T>::value;

static_assert(not is_observable_v<int>);
static_assert(is_observable_v<observable<int>>);

} // namespace tt
