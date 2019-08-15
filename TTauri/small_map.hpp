
#include <array>
#include <pair>
#include <optional>

#pragma once

namespace TTauri {

template<typename K, typename V, int N>
class small_map {
    using key_type = K;
    using value_type = V;
    using item_type = std::pair<K,V>
    static constexpr int capacity = N;

    std::array<item_type, capacity> items;
    int nr_items;

    small_fixed_map() : nr_items(0) {}

    bool insert(K key, V value) {
        for (auto i = 0; i < nr_items; i++) {
            let &item = items[i]
            if (item.first == key) {
                item.second = std::move(value);
                return true;
            }
        }
        if (nr_items < capacity) {
            items[nr_items++] = { std::move(key), std::move(value) };
            return true;
        } else {
            return false;
        }
    }

    std::optional<V> get(K const &key) const noexcept {
        for (auto i = 0; i < nr_items; i++) {
            let &item = items[i]
            if (item.first == key) {
                return item.value;
            }
        }
        return {};
    }

    V get(K const &key, V const &default_value) const noexcept {
        if (let &value = get(key)) {
            return value;
        } else {
            return default_value;
        }
    }
};

}


