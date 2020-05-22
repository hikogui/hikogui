// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include <mutex>
#include <vector>
#include <tuple>
#include <functional>

namespace TTauri {


template<typename... Args>
struct notifier {
public:
    using callback_type = std::function<void(Args const &...)>;

private:
    mutable std::mutex mutex;
    size_t counter = 0;
    std::vector<std::pair<size_t,callback_type>> callbacks;

public:
    size_t add(callback_type callback) noexcept {
        auto lock = std::scoped_lock(mutex);

        auto id = ++counter;
        callbacks.emplace_back(id, std::move(callback));
        return id;
    }

    size_t add_and_call(callback_type callback, Args const &... args) noexcept {
        let id = add(callback);
        callback(args...);
        return id;
    }

    void remove(size_t id) noexcept {
        auto lock = std::scoped_lock(mutex);

        let new_end = std::remove_if(callbacks.begin(), callbacks.end(), [id](let &item) {
            return item.first == id;
        });
        callbacks.erase(new_end, callbacks.cend());
    }

    void operator()(Args const &... args) noexcept {
        auto lock = std::scoped_lock(mutex);

        for (let &[id, callback] : callbacks) {
            callback(args...);
        }
    }

};

}