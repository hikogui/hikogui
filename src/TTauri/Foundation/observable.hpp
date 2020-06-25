// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/notifier.hpp"
#include "TTauri/Foundation/cpu_utc_clock.hpp"
#include "TTauri/Foundation/fast_mutex.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include <memory>
#include <functional>
#include <algorithm>

namespace tt {

namespace detail {

/** Observable abstract base class.
 * Objects of the observable_base class will notify listeners through
 * callbacks of changes of its value.
 *
 * This class does not hold the value itself, concrete subclasses will
 * either hold the value or calculate the value on demand. In many cases
 * concrete subclasses may be sub-expressions of other observable objects.
 *
 * This object will also track the time when the value was last modified
 * so that the value can be animated. Usefull when displaying the value
 * as an animated graphic element. For calculating inbetween values
 * it will keep track of the previous value.
 *
 */
template<typename T>
class observable_base {
public:
    using callback_type = std::function<void(T const &)>;
    using time_point = typename hires_utc_clock::time_point;
    using duration = typename hires_utc_clock::duration;

protected:
    mutable fast_mutex mutex;

private:
    T _previous_value;
    time_point _last_modified;
    notifier<T> notifier;

public:
    observable_base(observable_base const &) = delete;
    observable_base(observable_base &&) = delete;
    observable_base &operator=(observable_base const &) = delete;
    observable_base &operator=(observable_base &&) = delete;
    virtual ~observable_base() = default;

    /** Constructor.
     * @param initial_value The initial value used as the start position of the first animation.
     */
    observable_base(T const &initial_value) noexcept :
        _previous_value(initial_value), _last_modified(), notifier() {}

    /** Get the previous value
    */
    [[nodiscard]] T previous_value() const noexcept {
        ttlet lock = std::scoped_lock(mutex);
        return _previous_value;
    }

    /** Time when the value was modified last.
    */
    [[nodiscard]] time_point time_when_last_modified() const noexcept {
        ttlet lock = std::scoped_lock(mutex);
        return _last_modified;
    }

    /** Duration since the value was last modified.
    */
    [[nodiscard]] duration duration_since_last_modified() const noexcept {
        return cpu_utc_clock::now() - time_when_last_modified();
    }

    /** The relative time since the start of the animation.
    * The relative time since the start of the animation according to the duration of the animation.
    *
    * @param A relative value between 0.0 and 1.0.
    */
    [[nodiscard]] float animation_progress(duration animation_duration) const noexcept {
        tt_assume(animation_duration.count() != 0);

        return std::clamp(
            numeric_cast<float>(duration_since_last_modified().count()) / numeric_cast<float>(animation_duration.count()),
            0.0f,
            1.0f
        );
    }

    /** Get the current value.
     * The value is often calculated directly from the cached values retrieved from
     * notifications down the chain.
     */
    [[nodiscard]] virtual T load() const noexcept = 0;

    /** Get the current value animated over the animation_duration.
    */
    [[nodiscard]] T load(duration animation_duration) const noexcept {
        return mix(animation_progress(animation_duration), previous_value(), load());
    }

    /** Set the value.
     * The value is often not directly stored, but instead forwarded up
     * the chain of observables. And then let the notifications flowing backward
     * updated the cached values so that loads() will be quick.
     *
     * @param new_value The new value
     */
    virtual void store(T const &new_value) noexcept = 0;

    /** Add a callback as a listener.
     * @param callback The callback to register, the new value is passed as the first argument.
     * @return The id of the callback, used to unregister the callback.
     */
    [[nodiscard]] size_t add_callback(callback_type callback) noexcept {
        return notifier.add(callback);
    }

    /** Remove a callback.
     * @param id The id of the callback returned by the `add_callback()` method.
     */
    void remove_callback(size_t id) noexcept {
        notifier.remove(id);
    }

protected:

    /** Notify listeners of a change in value.
     * This function is used to notify listeners of this observable and also
     * to keep track of the previous value and start the animation.
     */
    void notify(T const &old_value, T const &new_value) noexcept {
        {
            ttlet lock = std::scoped_lock(mutex);
            _previous_value = old_value;
            _last_modified = cpu_utc_clock::now();
        }
        notifier(new_value);
    }
};

template<typename T>
class observable_value final : public observable_base<T> {
private:
    T value;

public:
    observable_value() noexcept :
        observable_base<T>(T{}), value() {}

    observable_value(T const &value) noexcept :
        observable_base<T>(value), value(value) {}

    virtual T load() const noexcept override {
        ttlet lock = std::scoped_lock(observable_base<T>::mutex);
        return value;
    }

    virtual void store(T const &new_value) noexcept override {
        T old_value;
        {
            ttlet lock = std::scoped_lock(observable_base<T>::mutex);
            old_value = value;
            value = new_value;
        }
        this->notify(old_value, new_value);
    }
};

template<typename T, typename OT>
class observable_unari : public observable_base<T> {
protected:
    std::shared_ptr<observable_base<OT>> operand;
    OT operand_cache;
    size_t operand_cb_id;

public:
    observable_unari(T const &initial_value, std::shared_ptr<observable_base<OT>> const &operand) noexcept :
        observable_base<T>(initial_value),
        operand(operand),
        operand_cache(operand->load())
    {
        operand_cb_id = this->operand->add_callback([this](OT const &value) {
            ttlet old_value = this->load();
            {
                ttlet lock = std::scoped_lock(observable_base<T>::mutex);
                operand_cache = value;
            }
            ttlet new_value = this->load();
            notify(old_value, new_value);
        });
    }

    ~observable_unari() {
        operand->remove_callback(operand_cb_id);
    }
};

template<typename OT>
class observable_not final : public observable_unari<bool,OT> {
public:
    observable_not(std::shared_ptr<observable_base<OT>> const &operand) noexcept :
        observable_unari<bool,OT>(!operand->load(), operand) {}

    virtual bool load() const noexcept override {
        ttlet lock = std::scoped_lock(observable_unari<bool,OT>::mutex);
        return !this->operand_cache;
    }

    virtual void store(bool const &new_value) noexcept override {
        this->operand->store(static_cast<OT>(!new_value));
    }
};

}

template<typename T>
class observable {
public:
    using value_type = T;
    using callback_type = std::function<void(value_type const &)>;
    using time_point = hires_utc_clock::time_point;
    using duration = hires_utc_clock::duration;

private:
    std::shared_ptr<detail::observable_base<value_type>> pimpl;

    observable(std::shared_ptr<detail::observable_base<value_type>> const &value) noexcept :
        pimpl(value) {}

    observable(std::shared_ptr<detail::observable_base<value_type>> &&value) noexcept :
        pimpl(std::move(value)) {}

public:
    observable(observable const &other) noexcept = default;
    observable(observable &&other) noexcept = default;
    observable &operator=(observable const &other) noexcept = default;
    observable &operator=(observable &&other) noexcept = default;

    observable() noexcept :
        observable(std::make_shared<detail::observable_value<value_type>>()) {}

    observable(value_type const &value) noexcept :
        observable(std::make_shared<detail::observable_value<value_type>>(value)) {}

    [[nodiscard]] value_type previous_value() const noexcept {
        tt_assume(pimpl);
        return pimpl->previous_value();
    }

    /** Time when the value was modified last.
    */
    [[nodiscard]] time_point time_when_last_modified() const noexcept {
        tt_assume(pimpl);
        return pimpl->time_when_last_modified();
    }

    /** Duration since the value was last modified.
    */
    [[nodiscard]] duration duration_since_last_modified() const noexcept {
        tt_assume(pimpl);
        return pimpl->duration_since_last_modified();
    }

    /** The relative time since the start of the animation.
    * The relative time since the start of the animation according to the duration of the animation.
    *
    * @param A relative value between 0.0 and 1.0.
    */
    [[nodiscard]] float animation_progress(duration animation_duration) const noexcept {
        tt_assume(pimpl);
        return pimpl->animation_progress(animation_duration);
    }

    [[nodiscard]] bool animating(duration animation_duration) const noexcept {
        tt_assume(pimpl);
        return pimpl->animation_progress(animation_duration) < 1.0f;
    }

    [[nodiscard]] value_type load() const noexcept {
        tt_assume(pimpl);
        return pimpl->load();
    }

    [[nodiscard]] value_type operator*() const noexcept {
        tt_assume(pimpl);
        return pimpl->load();
    }

    [[nodiscard]] value_type load(duration animation_duration) const noexcept {
        tt_assume(pimpl);
        return pimpl->load(animation_duration);
    }

    void store(value_type const &new_value) noexcept {
        tt_assume(pimpl);
        return pimpl->store(new_value);
    }

    observable &operator=(value_type const &value) noexcept {
        store(value);
        return *this;
    }

    [[nodiscard]] size_t add_callback(callback_type callback) noexcept {
        tt_assume(pimpl);
        return pimpl->add_callback(callback);
    }

    void remove_callback(size_t id) noexcept {
        tt_assume(pimpl);
        return pimpl->remove_callback(id);
    }

    [[nodiscard]] friend observable<bool> operator!(observable const &rhs) noexcept {
        return std::make_shared<detail::observable_not<bool>>(rhs.pimpl);
    }

    [[nodiscard]] friend bool operator==(observable const &lhs, value_type const &rhs) noexcept {
        return *lhs == rhs;
    }

    [[nodiscard]] friend bool operator==(value_type const &lhs, observable const &rhs) noexcept {
        return lhs == *rhs;
    }

    [[nodiscard]] friend bool operator!=(observable const &lhs, value_type const &rhs) noexcept {
        return *lhs != rhs;
    }

    [[nodiscard]] friend bool operator!=(value_type const &lhs, observable const &rhs) noexcept {
        return lhs != *rhs;
    }

    [[nodiscard]] friend float to_float(observable const &rhs) noexcept {
        return numeric_cast<float>(rhs.load());
    }

    [[nodiscard]] friend float to_float(observable const &rhs, duration animation_duration) noexcept {
        ttlet previous_value = numeric_cast<float>(rhs.previous_value());
        ttlet current_value = numeric_cast<float>(rhs.load());
        ttlet animation_progress = rhs.animation_progress(animation_duration);
        return mix(animation_progress, previous_value, current_value);
    }

};

}

