// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "interval.hpp"
#include "alignment.hpp"
#include <vector>
#include <optional>

namespace tt {

enum class flow_resistance {
    normal,
    greedy,
    resist
};

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
        cache_extent = {};
    }

    [[nodiscard]] size_t size() const noexcept
    {
        return items.size();
    }

    void
    update(ssize_t index, finterval extent, flow_resistance resistance, float margin, relative_base_line base_line) noexcept
    {
        tt_assume(index >= 0);

        cache_extent = {};

        grow(index + 1);
        items[index].update(extent, resistance, base_line);
        margins[index] = std::max(margins[index], margin);
        margins[index+1] = std::max(margins[index+1], margin);
    }

    /** Calculate the size of the combined items in the layout.
     */
    [[nodiscard]] finterval extent() const noexcept {
        if (cache_extent) {
            return *cache_extent;
        }

        auto r = finterval{0.0f};

        for (ttlet &margin: margins) {
            r += margin;
        }
        for (ttlet &item: items) {
            r += item.extent();
        }

        cache_extent = r;
        return r;
    }

    void flow_default() noexcept {
        for (auto &&item: items) {
            item.size = std::ceil(item.extent().minimum());
        }
    }

    [[nodiscard]] ssize_t flow_non_max(flow_resistance resistance) const noexcept
    {
        auto nr_non_max = ssize_t{0};

        for (auto &&item : items) {
            if (item.resistance == resistance && item.size < item.extent()) {
                ++nr_non_max;
            }
        }
        return nr_non_max;
    }

    [[nodiscard]] ssize_t flow_expand(ssize_t nr_non_max, flow_resistance resistance, float &extra_size) noexcept
    {
        ttlet extra_size_per_item = std::ceil(extra_size / nr_non_max);

        nr_non_max = 0;
        for (auto &&item : items) {
            if (item.resistance == resistance) {
                auto old_size = item.size;

                ttlet extra_size_this_item = std::min(extra_size, extra_size_per_item);

                item.size = std::ceil(clamp(item.size + extra_size_this_item, item.extent()));
                extra_size -= item.size - old_size;

                if (item.size < item.extent()) {
                    ++nr_non_max;
                }
            }
        }
        return nr_non_max;
    }

    void flow_positions() noexcept
    {
        auto offset = 0.0f;
        for (ssize_t i = 0; i != ssize(items); ++i) {
            offset += margins[i];
            items[i].offset = std::floor(offset);
            offset += items[i].size;
        }
    }

    void flow(float total_size) noexcept {
        tt_assume(cache_extent);
        auto minimum_size = extent().minimum();
        auto extra_size = total_size - minimum_size;

        flow_default();

        auto nr_non_max = flow_non_max(flow_resistance::greedy);
        while (extra_size >= 1.0f && nr_non_max != 0) {
            nr_non_max = flow_expand(nr_non_max, flow_resistance::greedy, extra_size);
        }

        nr_non_max = flow_non_max(flow_resistance::normal);
        while (extra_size >= 1.0f && nr_non_max != 0) {
            nr_non_max = flow_expand(nr_non_max, flow_resistance::normal, extra_size);
        }

        nr_non_max = flow_non_max(flow_resistance::resist);
        while (extra_size >= 1.0f && nr_non_max != 0) {
            nr_non_max = flow_expand(nr_non_max, flow_resistance::resist, extra_size);
        }

        flow_positions();
    }

   
    /**
     * @param first The first index
     * @param last The index one beyond the last.
     */
    [[nodiscard]] std::pair<float, float> get_offset_and_size(ssize_t first, ssize_t last) const noexcept
    {
        tt_assume(first >= 0 && first < ssize(items));
        tt_assume(last > 0 && last <= ssize(items));

        auto offset = items[first].offset;
        auto size = (items[last - 1].offset + items[last - 1].size) - offset;
        return {offset, size};
    }

    [[nodiscard]] std::pair<float, float> get_offset_and_size(ssize_t index) const noexcept
    {
        return get_offset_and_size(index, index + 1);
        
    }

    [[nodiscard]] relative_base_line get_base_line(ssize_t index) const noexcept
    {
        tt_assume(index >= 0 && index < ssize(items));
        return items[index].base_line;
    }

private:
    void grow(ssize_t new_size) noexcept
    {
        if (new_size > std::ssize(items)) {
            items.resize(new_size);
            margins.resize(new_size + 1);
        }
        tt_assume(margins.size() == items.size() + 1);
    }

    struct flow_layout_item {
        constexpr flow_layout_item() noexcept : _extent(), resistance(flow_resistance::normal), base_line() {}

        constexpr flow_layout_item(flow_layout_item const &rhs) noexcept = default;
        constexpr flow_layout_item(flow_layout_item &&rhs) noexcept = default;
        constexpr flow_layout_item &operator=(flow_layout_item const &rhs) noexcept = default;
        constexpr flow_layout_item &operator=(flow_layout_item &&rhs) noexcept = default;

        constexpr void update(finterval a_extent, flow_resistance _resistance, relative_base_line _base_line) noexcept
        {
            _extent = intersect(_extent, a_extent);
            resistance = std::max(resistance, _resistance);
            base_line = std::max(base_line, _base_line);
        }

        constexpr finterval extent() const noexcept
        {
            return _extent;
        }

        finterval _extent;
        flow_resistance resistance;
        relative_base_line base_line;

        float offset;
        float size;
    };

    /* The margin between the items, margin[0] is the margin
     * before the first item. margin[items.size()] is the margin
     * after the last item. margins.size() == items.size() + 1.
     */
    std::vector<float> margins;
    std::vector<flow_layout_item> items;

    mutable std::optional<finterval> cache_extent;
};

}
