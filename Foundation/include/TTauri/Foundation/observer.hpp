// Copyright 2020 Pokitec
// All rights reserved.

#pragma once
#include "TTauri/Foundation/observed.hpp"
#include <functional>
#include <vector>
#include <atomic>
#include <mutex>
#include <algorithm>

namespace TTauri {


template<typename T>
class observer {
public:
    using observed_type = observed<T>;
    using value_type = typename observed_type::value_type;
    using callback_type = typename observed_type::callback_type;
    using handle_type = typename observed_type::handle_type;

private:
    /** A reference to the object to be observed.
     */
    observed_type &observed_object;

    /** A handle to the callback registred with observed_object.
     * This handle is used in the destructor to unregister the callback.
     */
    handle_type observed_handle;

public:
    observer(observer const &) = delete;
    observer(observer &&) = delete;
    observer &operator=(observer const &) = delete;
    observer &operator=(observer &&) = delete;

    ~observer() {
        observed_object.unregister_callback(observed_handle);
    }

    observer(observed_type &value, callback_type callback) noexcept :
        observed_object(value), observed_handle(observed_object.register_callback(callback)) {}

    operator value_type () const noexcept {
        return static_cast<value_type>(observed_object);
    }

    observer &operator=(value_type const &rhs) noexcept {
        observed_object = rhs;
        return *this;
    }

    observer &operator=(value_type &&rhs) noexcept {
        observed_object = std::move(rhs);
        return *this;
    }
};

}