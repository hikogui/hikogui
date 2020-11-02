// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "observable_base.hpp"
#include <concepts>

namespace tt::detail {

template<typename T>
class observable_value final : public observable_base<T> {
public:
    using super = observable_base<T>;

    observable_value() noexcept :
        super(), _value() {}

    observable_value(T const &value) noexcept :
        super(), _value(value) {}

    T load() const noexcept override {
        ttlet lock = std::scoped_lock(this->_mutex);
        return _value;
    }

    bool store(T const &new_value) noexcept override {
        this->_mutex.lock();

        ttlet old_value = _value;
        if constexpr (std::equality_comparable<T>) {
            if (new_value != old_value) {
                _value = new_value;
                this->_mutex.unlock();
                this->notify(old_value, new_value);
                return true;
            } else {
                this->_mutex.unlock();
                return false;
            }

        } else {
            _value = new_value;
            this->_mutex.unlock();
            this->notify(old_value, new_value);
            return true;
        }
    }

private:
    T _value;
};

}