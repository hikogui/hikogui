// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "interval.hpp"
#include "alignment.hpp"
#include "ranged_numeric.hpp"
#include <vector>
#include <optional>

namespace tt {

/** Layout algorithm.
 */
class flow_layout {
public:
    flow_layout() noexcept : margins(), items() {
        clear();
    }

    void clear() noexcept
    {
        margins.clear();
        margins.push_back(0.0f);
        items.clear();
    }

    [[nodiscard]] size_t nr_items() const noexcept
    {
        return items.size();
    }

    void
    update(ssize_t index, float extent, ranged_int<3> resistance, float margin) noexcept
    {
        tt_axiom(index >= 0);
        tt_axiom(index < std::ssize(items));
        tt_axiom(index + 1 < std::ssize(margins));
        items[index].update(extent, resistance);
        margins[index] = std::max(margins[index], margin);
        margins[index+1] = std::max(margins[index+1], margin);
    }

    [[nodiscard]] float minimum_size() const noexcept
    {
        auto a = std::accumulate(margins.cbegin(), margins.cend(), 0.0f);
        return std::accumulate(items.cbegin(), items.cend(), a, [](ttlet &acc, ttlet &item) {
            return acc + item.minimum_size();
        });
    }

    /** Update the layout of all items based on the total size.
     */
    void set_size(float total_size) noexcept {
        set_items_to_minimum_size();
        auto extra_size = total_size - size();

        for (ttlet resistance : ranged_int<3>::range()) {
            auto nr_can_grow = number_of_items_that_can_grow(resistance);
            while (extra_size >= 1.0f && nr_can_grow != 0) {
                nr_can_grow = grow_items(nr_can_grow, resistance, extra_size);
            }
        }

        calculate_offset_and_size();
    }

    /** Calculate the size of the combined items in the layout.
     * @pre `set_size()` must be called.
     */
    [[nodiscard]] float size() const noexcept
    {
        if (items.empty()) {
            return margins.back();
        } else {
            tt_axiom(items.back().offset >= 0.0f);
            return items.back().offset + items.back().size + margins.back();
        }
    }

    /**
     * @param first The first index
     * @param last The index one beyond the last.
     */
    [[nodiscard]] std::pair<float, float> get_offset_and_size(ssize_t first, ssize_t last) const noexcept
    {
        tt_axiom(first >= 0 && first < std::ssize(items));
        tt_axiom(last > 0 && last <= std::ssize(items));

        auto offset = items[first].offset;
        auto size = (items[last - 1].offset + items[last - 1].size) - offset;
        return {offset, size};
    }

    [[nodiscard]] std::pair<float, float> get_offset_and_size(ssize_t index) const noexcept
    {
        return get_offset_and_size(index, index + 1);
        
    }

    /** Grow layout to include upto new_size of items.
     */
    void reserve(ssize_t new_size) noexcept
    {
        while (std::ssize(items) < new_size) {
            items.emplace_back();
        }

        while (std::ssize(margins) < new_size + 1) {
            margins.push_back(0.0f);
        }

        tt_axiom(margins.size() == items.size() + 1);
    }

private:
    struct flow_layout_item {
        constexpr flow_layout_item() noexcept : _minimum_size(0.0f), _resistance(1), offset(-1.0f), size(-1.0f) {}

        constexpr flow_layout_item(flow_layout_item const &rhs) noexcept = default;
        constexpr flow_layout_item(flow_layout_item &&rhs) noexcept = default;
        constexpr flow_layout_item &operator=(flow_layout_item const &rhs) noexcept = default;
        constexpr flow_layout_item &operator=(flow_layout_item &&rhs) noexcept = default;

        constexpr void update(float minimum_size, ranged_int<3> resistance) noexcept
        {
            _minimum_size = std::max(_minimum_size, minimum_size);
            switch (static_cast<int>(resistance)) {
            case 0: // No resistance has lower priority than strong resistance.
                _resistance = _resistance == 2 ? 2 : 0;
                break;
            case 1: // Normal resistance will not change the value.
                break;
            case 2: // Strong resistance overrides all
                _resistance = 2;
                break;
            default:
                tt_no_default();
            }
        }

        [[nodiscard]] float minimum_size() const noexcept {
            return _minimum_size;
        }

        [[nodiscard]] float maximum_size() const noexcept {
            return std::numeric_limits<float>::infinity();
        }

        [[nodiscard]] ranged_int<3> resistance() const noexcept
        {
            return _resistance;
        }

        float offset;
        float size;

    private:
        float _minimum_size;
        ranged_int<3> _resistance;
    };

    /* The margin between the items, margin[0] is the margin
     * before the first item. margin[items.size()] is the margin
     * after the last item. margins.size() == items.size() + 1.
     */
    std::vector<float> margins;
    std::vector<flow_layout_item> items;

    void set_items_to_minimum_size() noexcept
    {
        for (auto &&item : items) {
            item.size = std::ceil(item.minimum_size());
        }
        calculate_offset_and_size();
    }

    [[nodiscard]] ssize_t number_of_items_that_can_grow(ranged_int<3> resistance) const noexcept
    {
        auto nr_non_max = ssize_t{0};

        for (auto &&item : items) {
            if (item.resistance() == resistance && item.size < item.maximum_size()) {
                ++nr_non_max;
            }
        }
        return nr_non_max;
    }

    [[nodiscard]] ssize_t grow_items(ssize_t nr_non_max, ranged_int<3> resistance, float &extra_size) noexcept
    {
        ttlet extra_size_per_item = std::ceil(extra_size / nr_non_max);

        nr_non_max = 0;
        for (auto &&item : items) {
            if (item.resistance() == resistance) {
                auto old_size = item.size;

                ttlet extra_size_this_item = std::min(extra_size, extra_size_per_item);

                item.size = std::ceil(item.size + extra_size_this_item);
                extra_size -= item.size - old_size;

                if (item.size < item.maximum_size()) {
                    ++nr_non_max;
                }
            }
        }
        return nr_non_max;
    }

    void calculate_offset_and_size() noexcept
    {
        auto offset = 0.0f;
        for (ssize_t i = 0; i != std::ssize(items); ++i) {
            offset += margins[i];
            items[i].offset = std::floor(offset);
            offset += items[i].size;
        }
    }
};

}
