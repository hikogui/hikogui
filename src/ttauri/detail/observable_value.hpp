// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "observable_base.hpp"
#include <concepts>

namespace tt::detail {

template<typename T>
class observable_value final : public observable_base<T> {
public:
    observable_value() noexcept :
        observable_base<T>(), _value() {}

    observable_value(T const &value) noexcept :
        observable_base<T>(), _value(value) {}

    virtual T load() const noexcept override {
        ttlet lock = std::scoped_lock(observable_base<T>::_mutex);
        return _value;
    }

    virtual bool store(T const &new_value) noexcept override {
        observable_base<T>::_mutex.lock();

        ttlet old_value = _value;
        if constexpr (std::equality_comparable<T>) {
            if (new_value != old_value) {
                _value = new_value;
                observable_base<T>::_mutex.unlock();
                this->notify(old_value, new_value);
                return true;
            } else {
                observable_base<T>::_mutex.unlock();
                return false;
            }

        } else {
            _value = new_value;
            observable_base<T>::_mutex.unlock();
            this->notify(old_value, new_value);
            return true;
        }
    }

private:
    T _value;
};

}