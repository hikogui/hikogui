// Copyright 2020 Pokitec
// All rights reserved.

#pragma once
#include <functional>
#include <vector>
#include <atomic>
#include <mutex>
#include <algorithm>

namespace TTauri {


template<typename T>
class observed {
public:
    using value_type = T;
    using callback_type = std::function<void(T)>;
    using handle_type = uint64_t;

private:
    mutable std::mutex mutex;

    std::atomic<value_type> intrinsic = value_type{};
    handle_type handle_counter = 0;
    std::vector<std::pair<handle_type,callback_type>> callbacks;

public:
    observed() noexcept {}
    observed(observed const &) = delete;
    observed(observed &&) = delete;
    observed &operator=(observed const &) = delete;
    observed &operator=(observed &&) = delete;

    observed(value_type rhs) noexcept : intrinsic(std::move(rhs)) {}

    observed &operator=(value_type const &rhs) noexcept {
        intrinsic = rhs;
        notify_observers(intrinsic);
        return *this;
    }

    observed &operator=(value_type &&rhs) noexcept {
        intrinsic = std::move(rhs);
        notify_observers(intrinsic);
        return *this;
    }

    operator T() const noexcept {
        return intrinsic;
    }

    [[nodiscard]] handle_type register_callback(callback_type callback) noexcept {
        ttauri_assume(static_cast<bool>(callback));

        auto lock = std::scoped_lock(mutex);
        auto handle = ++handle_counter;
        callbacks.emplace_back(handle, std::move(callback));
        return handle;
    }

    void unregister_callback(handle_type handle) noexcept {
        auto lock = std::scoped_lock(mutex);
        auto new_end = std::remove_if(callbacks.begin(), callbacks.end(), [=](let &x) {
            return x.first == handle;
        });
        ttauri_assume(new_end != callbacks.cend());
        callbacks.erase(new_end, callbacks.cend());
    }

private:
    void notify_observers(value_type const &rhs) const noexcept {
        auto lock = std::scoped_lock(mutex);

        for (let [handle, callback] : callbacks) {
            callback(rhs);
        }
    }

};

}
