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


/** @file observer.hpp
 *
 * an observer is an object that observes an obexpr:
 *  - An observer is the (shared) owner of the obexpr tree.
 *  - Notification from the obexpr will:
 *    - Update the cached value.
 *    - Execute callbacks registered with the observer
 *  - The cached value can be read through the conversion operator.
 *  - Any write or read/modify/write operation will be forwarded to the expression.
 *  - Any operation on the observer will return a copy of the cached or computed value.
 *
 * an observable is an object which is observed by the obexpr_observer:
 *  - Any write or read/modify/write operation will cause a notifaction to
 *    be send to any registered obexpr_observer.
 *  - Any operation on the observable will return a copy of the cached or computed value.
 *  - To turn a observable to a obexpr, use the expr() function.
 *  
 * A obexpr is a expression object that forms a tree of other obexpr objects.
 *  - When a leaf value changes, its value is cashed and notification is send
 *    through the tree to the root object.
 *  - When a observer is a (shared) owner of a obexpr it will be notified with
 *    the calculated value of the expression.
 *  - certain obexpr objects can forward write or read/modify/write operation
 *    toward the leaf nodes.
 *  - leaf nodes can forward write or read/modify/write operations to the observable.
 */

namespace tt {

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
    */
    [[nodiscard]] virtual T load() const noexcept = 0;

    /** Get the current value animated over the animation_duration.
    */
    [[nodiscard]] T load(duration animation_duration) const noexcept {
        return mix(animation_progress(animation_duration), previous_value(), load());
    }

    virtual void store(T const &new_value) noexcept = 0;

    void notify(T const &old_value, T const &new_value) noexcept {
        {
            ttlet lock = std::scoped_lock(mutex);
            _previous_value = old_value;
            _last_modified = cpu_utc_clock::now();
        }
        notifier(new_value);
    }

    [[nodiscard]] size_t add_callback(callback_type callback) noexcept {
        return notifier.add(callback);
    }

    void remove_callback(size_t id) noexcept {
        notifier.remove(id);
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
        notify(old_value, new_value);
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
            ttlet old_value = load();
            {
                ttlet lock = std::scoped_lock(observable_base<T>::mutex);
                operand_cache = value;
            }
            ttlet new_value = load();
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
        return !operand_cache;
    }

    virtual void store(bool const &new_value) noexcept override {
        operand->store(static_cast<OT>(!new_value));
    }
};

template<typename T>
class observable {
public:
    using value_type = T;
    using callback_type = std::function<void(value_type const &)>;
    using time_point = hires_utc_clock::time_point;
    using duration = hires_utc_clock::duration;

private:
    std::shared_ptr<observable_base<value_type>> pimpl;

public:
    observable(observable const &other) noexcept = default;
    observable(observable &&other) noexcept = default;
    observable &operator=(observable const &other) noexcept = default;
    observable &operator=(observable &&other) noexcept = default;

    observable(std::shared_ptr<observable_base<value_type>> const &value) noexcept :
        pimpl(value) {}

    observable(std::shared_ptr<observable_base<value_type>> &&value) noexcept :
        pimpl(std::move(value)) {}

    observable() noexcept :
        observable(std::make_shared<observable_value<value_type>>()) {}

    observable(value_type const &value) noexcept :
        observable(std::make_shared<observable_value<value_type>>(value)) {}

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
        return std::make_shared<observable_not<bool>>(rhs.pimpl);
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

