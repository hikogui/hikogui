// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <array>
#include <utility>
#include <optional>

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

private:
    std::array<item_type, capacity> items;
    int nr_items;

public:
    small_map() : nr_items(0) {}

    small_map(small_map const &other) : nr_items(0) {
        for (let &item: other) {
            items[nr_items++] = { item.key, item.value };
        }
    }

    small_map(small_map &&other) : nr_items(0) {
        using namespace std;
  
        for (auto &item: other) {
            swap(items[nr_items++], item);
        }
        other.nr_items = 0;
    }

    small_map &operator=(small_map const &other) {
        nr_items = 0;
        for (let &item: other) {
            items[nr_items++] = item;
        }
        return *this;
    }

    small_map &operator=(small_map &&other) {
        using namespace std;

        nr_items = 0;
        for (let &item: other) {
            swap([nr_items++], item);
        }
        other.nr_items = 0;
        return *this;
    }

    int size() const {
        return nr_items;
    }

    decltype(auto) begin() const { return items.begin(); }
    decltype(auto) begin() { return items.begin(); }
    decltype(auto) end() const { return items.begin() + nr_items; }
    decltype(auto) end() { return items.begin() + nr_items; }

    bool push(K &&key, V &&value) noexcept {
        if (nr_items < capacity) {
            items[nr_items++] = { std::forward<K>(key), std::forward<V>(value) };
            return true;
        } else {
            return false;
        }
    }

    std::optional<item_type> pop() noexcept {
        if (nr_items > 0) {
            return std::move(items[--nr_items]);
        } else {
            return {};
        }
    }

    bool insert(K &&key, V &&value) {
        for (auto i = 0; i < nr_items; i++) {
            auto &item = items[i];
            if (item.key == key) {
                item.value = std::forward<V>(value);
                return true;
            }
        }
        return push(std::forward<K>(key), std::forward<V>(value));
    }

    std::optional<V> get(K const &key) const noexcept {
        for (auto i = 0; i < nr_items; i++) {
            let &item = items[i];
            if (item.key == key) {
                return item.value;
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
};

}


