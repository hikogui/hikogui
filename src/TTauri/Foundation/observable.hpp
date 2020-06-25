// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/notifier.hpp"
#include "TTauri/Foundation/cpu_utc_clock.hpp"
#include "TTauri/Foundation/fast_mutex.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
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

