// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "interval.hpp"
#include "alignment.hpp"
#include <vector>
#include <optional>

namespace tt {

struct flow_layout_item {
    constexpr flow_layout_item() noexcept :
        minimum_size(0), preferred_size(0), maximum_size(0), offset(0), size(0)
    {
    }

    constexpr flow_layout_item(flow_layout_item const &rhs) noexcept = default;
    constexpr flow_layout_item(flow_layout_item &&rhs) noexcept = default;
    constexpr flow_layout_item &operator=(flow_layout_item const &rhs) noexcept = default;
    constexpr flow_layout_item &operator=(flow_layout_item &&rhs) noexcept = default;

    constexpr void update(float min_size, float pref_size, float max_size) noexcept
    {
        minimum_size = std::max(minimum_size, narrow_cast<int>(std::ceil(min_size)));
        preferred_size = std::max(preferred_size, narrow_cast<int>(std::round(pref_size)));
        maximum_size = std::max(maximum_size, narrow_cast<int>(std::floor(max_size)));

        // The maximum size must be larger than the minimum size.
        maximum_size = std::max(maximum_size, minimum_size);

        // Try to keep the preferred size below the maximum.
        preferred_size = std::clamp(preferred_size, minimum_size, maximum_size);

        tt_axiom(minimum_size <= preferred_size);
        tt_axiom(preferred_size <= maximum_size);
    }

    int offset;
    int size;
    int minimum_size;
    int preferred_size;
    int maximum_size;
};

/** Layout algorithm.
 */
class flow_layout {
public:
    flow_layout() noexcept : margins(), items()
    {
        clear();
    }

    void clear() noexcept
    {
        margins.clear();
        margins.push_back(0);
        items.clear();
    }

    [[nodiscard]] size_t nr_items() const noexcept
    {
        return items.size();
    }

    void update(ssize_t index, float minimum_size, float preferred_size, float maximum_size, float margin) noexcept
    {
        tt_axiom(index >= 0);
        tt_axiom(index < std::ssize(items));
        tt_axiom(index + 1 < std::ssize(margins));

        items[index].update(minimum_size, preferred_size, maximum_size);

        ttlet margin_ = narrow_cast<int>(std::ceil(margin));
        margins[index] = std::max(margins[index], margin_);
        margins[index + 1] = std::max(margins[index + 1], margin_);
    }

    [[nodiscard]] int total_margin_size() const noexcept
    {
        return std::accumulate(margins.cbegin(), margins.cend(), 0);
    }

    [[nodiscard]] float minimum_size() const noexcept
    {
        auto a = total_margin_size();
        for (ttlet &item : items) {
            a += item.minimum_size;
        }
        return narrow_cast<float>(a);
    }

    [[nodiscard]] float preferred_size() const noexcept
    {
        auto a = total_margin_size();
        for (ttlet &item : items) {
            a += item.preferred_size;
        }
        return narrow_cast<float>(a);
    }

    [[nodiscard]] float maximum_size() const noexcept
    {
        auto a = total_margin_size();
        for (ttlet &item : items) {
            a += item.maximum_size;
        }
        return narrow_cast<float>(a);
    }

    /** Update the layout of all items based on the total size.
     */
    void set_size(float total_size) noexcept
    {
        ttlet total_size_ = narrow_cast<int>(std::round(total_size));
        // It is possible that total_size crosses the maximum_size.
        tt_axiom(total_size_ >= minimum_size());

        set_items_to_preferred_size();

        auto grow_by = total_size_ - size();
        while (grow_by != 0) { 
            int num = num_items_can_resize(grow_by);

            auto resize_beyond_maximum = num == 0;
            if (resize_beyond_maximum) {
                num = narrow_cast<int>(std::size(items));
            }

            resize_items(num, grow_by, resize_beyond_maximum);

            grow_by = total_size_ - size();
        };
    }

    [[nodiscard]] std::pair<float, float> get_offset_and_size(size_t index) const noexcept
    {
        tt_axiom(index < std::size(items));
        auto offset = narrow_cast<float>(items[index].offset);
        auto size = narrow_cast<float>(items[index].size);
        return {offset, size};
    }

    /** Grow layout to include upto new_size of items.
     */
    void reserve(ssize_t new_size) noexcept
    {
        while (std::ssize(items) < new_size) {
            items.emplace_back();
        }

        while (std::ssize(margins) < new_size + 1) {
            margins.push_back(0);
        }

        tt_axiom(margins.size() == items.size() + 1);
    }

private:
    /* The margin between the items, margin[0] is the margin
     * before the first item. margin[items.size()] is the margin
     * after the last item. margins.size() == items.size() + 1.
     */
    std::vector<int> margins;
    std::vector<flow_layout_item> items;

    void set_items_to_preferred_size() noexcept
    {
        for (auto &&item : items) {
            item.size = item.preferred_size;
        }
        calculate_offset_and_size();
    }

    [[nodiscard]] int num_items_can_resize(int grow_by) noexcept
    {
        if (grow_by > 0) {
            return narrow_cast<int>(std::ranges::count_if(items, [](ttlet &item) {
                return item.size < item.maximum_size;
            }));
        } else if (grow_by < 0) {
            return narrow_cast<int>(std::ranges::count_if(items, [](ttlet &item) {
                return item.size > item.minimum_size;
            }));

        } else {
            return 0;
        }
    }

    [[nodiscard]] void resize_items(int nr_items, int grow_by, bool resize_beyond_maximum) noexcept
    {
        tt_axiom(grow_by != 0);
        tt_axiom(nr_items > 0);

        auto per_item_grow_by = grow_by / nr_items;
        if (per_item_grow_by == 0) {
            per_item_grow_by = grow_by > 0 ? 1 : -1;
        }

        for (auto &&item : items) {
            auto new_item_size = item.size + per_item_grow_by;
            if (!resize_beyond_maximum) {
                new_item_size = std::clamp(new_item_size, item.minimum_size, item.maximum_size);
            }

            ttlet this_item_grown_by = new_item_size - item.size;
            item.size = new_item_size;

            tt_axiom(item.size >= item.minimum_size);
            if (!resize_beyond_maximum) {
                tt_axiom(item.size <= item.maximum_size);
            }

            if ((grow_by -= this_item_grown_by) == 0) {
                // All the growth has been spread to the widgets.
                break;
            }
        }

        calculate_offset_and_size();
    }

    void calculate_offset_and_size() noexcept
    {
        auto offset = 0;
        for (ssize_t i = 0; i != std::ssize(items); ++i) {
            offset += margins[i];
            items[i].offset = offset;
            offset += items[i].size;
        }
    }

    /** Calculate the size of the combined items in the layout.
     * @pre `calculate_offset_and_size()` must be called.
     */
    [[nodiscard]] int size() const noexcept
    {
        if (items.empty()) {
            return margins.back();
        } else {
            tt_axiom(items.back().offset >= 0);
            return items.back().offset + items.back().size + margins.back();
        }
    }
};

} // namespace tt
