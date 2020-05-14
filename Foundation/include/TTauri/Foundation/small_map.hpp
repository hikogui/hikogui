// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include <array>
#include <utility>
#include <optional>
#include <type_traits>

namespace TTauri {

template<typename K, typename V, int N>
class small_map {
public:
    using key_type = K;
    using value_type = V;
    struct item_type {
        K key;
        V value;
    };
    static constexpr int capacity = N;
    using array_type = std::array<item_type, capacity>;

private:
    typename array_type::iterator _end;
    array_type items = {};

public:
    small_map() {
        _end = items.begin();
    }

    small_map(small_map const &other) {
        ttauri_assume(this != &other);
        _end = items.begin();
        for (let &other_item: other) {
            auto &this_item = *(_end++);
            this_item = other_item;
        }
    }

    small_map(small_map &&other) {
        ttauri_assume(this != &other);
        using std::swap;
  
        _end = items.begin();
        for (auto &other_item: other) {
            auto &this_item = *(_end++);
            swap(this_item, other_item);
        }
        // All items in other are valid, no reason to set other._end.
    }

    small_map &operator=(small_map const &other) {
        if (this != &other) {
            _end = items.begin();
            for (let &other_item: other) {
                auto &this_item = *(_end++);
                this_item = other_item;
            }
        }
        return *this;
    }

    small_map &operator=(small_map &&other) {
        if (this != &other) {
            _end = items.begin();
            for (let &other_item: other) {
                auto &this_item = *(_end++);
                this_item = std::move(other_item);
            }
            other._end = other.begin();
        }
        return *this;
    }

    size_t size() const {
        return _end - items.begin();
    }

    decltype(auto) begin() const { return items.begin(); }
    decltype(auto) begin() { return items.begin(); }
    decltype(auto) end() const { return _end; }
    decltype(auto) end() { return _end; }

    template<typename O, typename P>
    bool push(O &&key, P &&value) noexcept {
        if (_end != items.end()) {
            auto &item = *(_end++);
            item.key = std::forward<O>(key);
            item.value = std::forward<P>(value);
            return true;
        } else {
            return false;
        }
    }

    std::optional<item_type> pop() noexcept {
        if (_end != items.begin()) {
            auto &item = *(--_end);
            return std::move(item);
        } else {
            return {};
        }
    }

    std::optional<V> get(K const &key) const noexcept {
        for (auto i = begin(); i != end(); ++i) {
            if (i->key == key) {
                return i->value;
            }
        }
        return {};
    }

    V get(K const &key, V const &default_value) const noexcept {
        if (let &value = get(key)) {
            return *value;
        } else {
            return default_value;
        }
    }

    V increment(K const &key) noexcept {
        static_assert(std::is_arithmetic_v<V>, "Only increment on a artihmatic value");
        auto i = begin();
        for (; i != end(); ++i) {
            if (i->key == key) {
                return ++(i->value);
            }
        }
        if (i != items.end()) {
            _end = i + 1;
            i->key = key;
            return i->value = V{1};
        }

        return 0;
    }
};

}


