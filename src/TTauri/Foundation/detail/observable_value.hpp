// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "observable_base.hpp"

namespace tt::detail {

template<typename T>
class observable_value final : public observable_base<T> {
private:
    T value;

public:
    observable_value() noexcept :
        observable_base<T>(), value() {}

    observable_value(T const &value) noexcept :
        observable_base<T>(), value(value) {}

    virtual T load() const noexcept override {
        ttlet lock = std::scoped_lock(observable_base<T>::mutex);
        return value;
    }

    virtual bool store(T const &new_value) noexcept override {
        observable_base<T>::mutex.lock();

        ttlet old_value = value;
        if (new_value != old_value) {
            value = new_value;
            observable_base<T>::mutex.unlock();
            this->notify(old_value, new_value);
            return true;
        } else {
            observable_base<T>::mutex.unlock();
            return false;
        }
    }
};

}