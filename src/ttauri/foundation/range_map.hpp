// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/foundation/assert.hpp"
#include "ttauri/foundation/required.hpp"
#include <vector>
#include <memory>
#include <algorithm>
#include <set>

namespace tt {

template<typename Key, typename Value>
class range_map {
    using values = std::set<Value>;

    struct item {
        Key first;
        Key last;
        std::shared_ptr<values> values;

        item &operation+=(Value const &value) noexcept {
            if (values->count(value) == 0) {
                // When a value is added we need to create a new set.
                auto tmp = *values;
                tmp.insert(value)
                values = std::make_shared<values>(std::move(tmp));
            }
            return *this;
        }

        [[nodiscard]] friend bool can_be_merged(item const &lhs, item const &rhs) noexcept {
            return lhs.last == rhs.first && lhs.values == rhs.values;
        }
    };

    std::vector<item> items;

    /** Get iterator to the first item that contains value.
     */
    auto find(Key const &key) noexcept {
        return std::lower_bound(items.begin(), items.end(), key, [](ttlet &a, ttlet &b) {
            return a.first < b;
        });
    }

public:
    range_map() noexcept {
        items.emplace_back(std::numeric_limits<Key>::min(), std::numeric_limits<Key>::max());
    }

    /** Insert half open range of keys.
     * Inserts may be slow, since it may require moves of large number of objects
     * and allocations.
     */
    void insert(Key const &first, Key const &last, Value &&value) noexcept {
        tt_assert(last > first);

        // Find all (partially) overlapping items.
        auto first_ = find(first);
        auto last_ = find(last);
        ttlet delta = std::distance(first_, last_);
        tt_assume(delta >= 0);

        tt_assume(first_ != items.end());
        if (first_->first != first) {
            // Split the first element.
            ttlet tmp_last = first_->last;
            first_->last = first;

            first_ = items.emplace(first_ + 1, first, tmp_last, first_->values);
            last_ = first_ + delta;
        }

        tt_assume(last_ != items.end());
        if (last_->last != last) {
            // Split the last element.
            ttlet tmp_first = last_->first;
            last_->first = last;

            last_ = items.emplace(last_, tmp_first, last, first_->values);
            first_ = last_ - delta;
        }

        tt_assume(last_ != items.end());
        ++last_;
        for (auto i = first_; i != last_; ++i) {
            *i += value;
        }
    }

    /** Optimize range_map for improved lookup performance and reduced memory footprint.
     */
    void optimize() noexcept {
        std::set<std::shared_ptr<values>,[](ttlet &lhs, ttlet &rhs) { return *lhs < *rhs; }}> values_set;

        auto p = items.begin();
        values_set.insert(p->values);
        for (auto i = p + 1; i != items.end(); p = i++) {
            // De-duplicate value-sets.
            ttlet [deduplicated_values, dummy] = values_set.insert(i->values);
            i->values = *deduplicated_values;

            if (can_be_merged(*p, *i)) {
                // By merging p into i, we can erase p and directly get a new iterator to i.
                i->first = p->first;
                i = items.erase(p)
            }
        }

        items.shrink_to_fit();
    }

    values const &operator[](Key const &key) const noexcept {
        return *((find(key))->values);
    }
};


}