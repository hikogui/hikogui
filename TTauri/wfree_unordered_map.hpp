// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <atomic>
#include <array>
#include <optional>

namespace TTauri {

template<typename K, typename V>
struct wait_free_unordered_map_item {
    /*! Hash for quick comparison and for state.
     * Special values:
     *  * 0 = Empty
     *  * 1 = Busy
     *  * 2 = Tombstone
     *
     * Natural hash values 0, 1, 2 must be mapped to 3, 4, 5.
     */
    alignas(16)
    std::atomic<V> value;
    std::atomic<size_t> hash = 0;
    K key;
};

template<size_t MAX_NR_ITEMS,typename K, typename V>
class wfree_unordered_map {
public:
    using key_type = K;
    using mapped_type = V;

private:
    static constexpr size_t CAPACITY = MAX_NR_ITEMS * 2;

    std::array<wait_free_unordered_map_item<K,V>, CAPACITY> items;

public:
    void insert(K key, V value) {
        let hash = std::hash<K>{}(key);
        let hash_compare = hash >= 3 ? hash : hash + 3;

        auto index = hash % CAPACITY;
        while (true) {
            auto &item = items[index];

            auto item_hash = item.hash.load(std::memory_order_acquire);

            if (item_hash == hash_compare && key == item.key) {
                // Key was already in map, replace the value.
                item.value.store(value, std::memory_order_release);
                return;

            } else if (item_hash == 0) {
                // Empty item.
                if (item.hash.compare_exchange_strong(item_hash, 1, std::memory_order_acquire)) { // Set to busy
                    item.key = std::move(key);
                    item.value.store(value, std::memory_order_relaxed);
                    item.hash.store(hash_compare, std::memory_order_release);
                    return;
                }
                // Another thread was ahead of us.
                // Even though compare_exchange is used here, this algorithm is wait-free since all threads
                // including the one here are making progress.
            }
            index = (index + 1) % CAPACITY;
        }
    }

    std::optional<V> get(K const &key) const {
        let hash = std::hash<K>{}(key);
        let hash_compare = hash >= 3 ? hash : hash + 3;

        auto index = hash % CAPACITY;
        while (true) {
            auto &item = items[index];

            auto item_hash = item.hash.load(std::memory_order_acquire);

            if (item_hash == hash_compare && key == item.key) {
                // Found key
                return { item.value };

            } else if (item_hash == 0) {
                // Item is empty.
                return {};
            }
            index = (index + 1) % CAPACITY;
        }
    }

    std::optional<V> erase(K const &key) {
        let hash = std::hash<K>{}(key);
        let hash_compare = hash >= 3 ? hash : hash + 3;

        auto index = hash % CAPACITY;
        while (true) {
            auto &item = items[index];
            auto item_hash = item.hash.load(std::memory_order_acquire);

            if (item_hash == hash_compare && key == item.key) {
                // Found key.
                let value = item.value
                // Set tombstone.
                item.hash.store(1, std::memory_order_release);
                item.key = {};
                item.value.store({}, std::memory_order_release);
                return { value };

            } else if (item_hash == 0) {
                // Item is empty.
                return;
            }
            index = (index + 1) % CAPACITY;
        }
    }
};


}