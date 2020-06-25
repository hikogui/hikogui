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

}