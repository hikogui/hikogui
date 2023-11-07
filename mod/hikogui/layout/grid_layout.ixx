// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <cstdint>
#include <numeric>
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>

export module hikogui_layout_grid_layout;
import hikogui_geometry;
import hikogui_layout_box_constraints;
import hikogui_layout_box_shape;
import hikogui_layout_spreadsheet_address;
import hikogui_utility;

export namespace hi { inline namespace v1 {
namespace detail {

template<typename T>
struct grid_layout_cell {
    using value_type = T;

    size_t first_column = 0;
    size_t first_row = 0;
    size_t last_column = 0;
    size_t last_row = 0;
    bool beyond_maximum = false;
    value_type value = {};
    box_shape shape = {};

    constexpr grid_layout_cell() noexcept = default;
    constexpr grid_layout_cell(grid_layout_cell const&) noexcept = default;
    constexpr grid_layout_cell(grid_layout_cell&&) noexcept = default;
    constexpr grid_layout_cell& operator=(grid_layout_cell const&) noexcept = default;
    constexpr grid_layout_cell& operator=(grid_layout_cell&&) noexcept = default;

    constexpr grid_layout_cell(
        size_t first_column,
        size_t first_row,
        size_t last_column,
        size_t last_row,
        bool beyond_maximum,
        std::convertible_to<value_type> auto&& value) noexcept :
        first_column(first_column),
        first_row(first_row),
        last_column(last_column),
        last_row(last_row),
        beyond_maximum(beyond_maximum),
        value(hi_forward(value))
    {
        hi_assert(first_column < last_column);
        hi_assert(first_row < last_row);
    }

    constexpr void set_constraints(box_constraints const& constraints) noexcept
    {
        _constraints = constraints;
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr size_t first() const noexcept
    {
        if constexpr (Axis == axis::x) {
            return first_column;
        } else if constexpr (Axis == axis::y) {
            return first_row;
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr size_t last() const noexcept
    {
        if constexpr (Axis == axis::x) {
            return last_column;
        } else if constexpr (Axis == axis::y) {
            return last_row;
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr size_t span() const noexcept
    {
        hi_axiom(first<Axis>() < last<Axis>());
        return last<Axis>() - first<Axis>();
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr auto alignment() const noexcept
    {
        if constexpr (Axis == axis::x) {
            return _constraints.alignment.horizontal();
        } else if constexpr (Axis == axis::y) {
            return _constraints.alignment.vertical();
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr float minimum() const noexcept
    {
        if constexpr (Axis == axis::x) {
            return _constraints.minimum.width();
        } else if constexpr (Axis == axis::y) {
            return _constraints.minimum.height();
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr float preferred() const noexcept
    {
        if constexpr (Axis == axis::x) {
            return _constraints.preferred.width();
        } else if constexpr (Axis == axis::y) {
            return _constraints.preferred.height();
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr float maximum() const noexcept
    {
        if constexpr (Axis == axis::x) {
            return _constraints.maximum.width();
        } else if constexpr (Axis == axis::y) {
            return _constraints.maximum.height();
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr float margin_before(bool forward) const noexcept
    {
        if constexpr (Axis == axis::x) {
            if (forward) {
                return _constraints.margins.left();
            } else {
                return _constraints.margins.right();
            }
        } else if constexpr (Axis == axis::y) {
            if (forward) {
                return _constraints.margins.bottom();
            } else {
                return _constraints.margins.top();
            }
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr float margin_after(bool forward) const noexcept
    {
        if constexpr (Axis == axis::x) {
            if (forward) {
                return _constraints.margins.right();
            } else {
                return _constraints.margins.left();
            }
        } else if constexpr (Axis == axis::y) {
            if (forward) {
                return _constraints.margins.top();
            } else {
                return _constraints.margins.bottom();
            }
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr float padding_before(bool forward) const noexcept
    {
        if constexpr (Axis == axis::x) {
            if (forward) {
                return _constraints.padding.left();
            } else {
                return _constraints.padding.right();
            }
        } else if constexpr (Axis == axis::y) {
            if (forward) {
                return _constraints.padding.bottom();
            } else {
                return _constraints.padding.top();
            }
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr float padding_after(bool forward) const noexcept
    {
        if constexpr (Axis == axis::x) {
            if (forward) {
                return _constraints.padding.right();
            } else {
                return _constraints.padding.left();
            }
        } else if constexpr (Axis == axis::y) {
            if (forward) {
                return _constraints.padding.top();
            } else {
                return _constraints.padding.bottom();
            }
        } else {
            hi_static_no_default();
        }
    }

private:
    box_constraints _constraints;
};

template<hi::axis Axis, typename T>
class grid_layout_axis_constraints {
public:
    constexpr static hi::axis axis = Axis;

    using value_type = T;
    using alignment_type = std::conditional_t<axis == axis::y, vertical_alignment, horizontal_alignment>;
    using cell_type = grid_layout_cell<value_type>;
    using cell_vector = std::vector<cell_type>;

    struct constraint_type {
        /** The minimum width/height of the cells.
         */
        float minimum = 0.0f;

        /** The preferred width/height of the cells.
         */
        float preferred = 0.0f;

        /** The maximum width/height of the cells.
         */
        float maximum = std::numeric_limits<float>::max();

        /** The left/top margin of the cells.
         */
        float margin_before = 0.0f;

        /** The right/bottom margin of the cells.
         */
        float margin_after = 0.0f;

        /** The left/top padding of the cells.
         */
        float padding_before = 0.0f;

        /** The right/bottom padding of the cells.
         */
        float padding_after = 0.0f;

        /** The alignment of the cells.
         */
        alignment_type alignment = alignment_type::none;

        /** Allow this cell to be resized beyond the maximum constraint.
         */
        bool beyond_maximum = false;

        /** The position of the cell.
         *
         * @note This field is valid after layout.
         */
        float position = 0.0f;

        /** Size of the cell.
         *
         * @note This field is valid after layout.
         */
        float extent = 0.0f;

        /** The before-position within this cell where to align to.
         *
         * @note This field is valid after layout.
         */
        std::optional<float> guideline = 0.0f;
    };
    using constraint_vector = std::vector<constraint_type>;
    using iterator = constraint_vector::iterator;
    using const_iterator = constraint_vector::const_iterator;
    using reverse_iterator = constraint_vector::reverse_iterator;
    using reference = constraint_vector::reference;
    using const_reference = constraint_vector::const_reference;

    constexpr ~grid_layout_axis_constraints() = default;
    constexpr grid_layout_axis_constraints() noexcept = default;
    constexpr grid_layout_axis_constraints(grid_layout_axis_constraints const&) noexcept = default;
    constexpr grid_layout_axis_constraints(grid_layout_axis_constraints&&) noexcept = default;
    constexpr grid_layout_axis_constraints& operator=(grid_layout_axis_constraints const&) noexcept = default;
    constexpr grid_layout_axis_constraints& operator=(grid_layout_axis_constraints&&) noexcept = default;
    [[nodiscard]] constexpr friend bool
    operator==(grid_layout_axis_constraints const&, grid_layout_axis_constraints const&) noexcept = default;

    /** Construct constraints for this axis.
     *
     * @param cells The cells
     * @param num The number of cells in the direction of the current axis
     * @param forward True if the axis is used from left-to-right or bottom-to-top,
     *                False if the axis is used from right-to-left or top-to-bottom.
     */
    constexpr grid_layout_axis_constraints(cell_vector const& cells, size_t num, bool forward) noexcept :
        _constraints(num), _forward(forward)
    {
        for (hilet& cell : cells) {
            construct_simple_cell(cell);
        }
        construct_fixup();

        for (hilet& cell : cells) {
            construct_span_cell(cell);
        }
        construct_fixup();
    }

    [[nodiscard]] constexpr float margin_before() const noexcept
    {
        return empty() ? 0 : _forward ? front().margin_before : back().margin_before;
    }

    [[nodiscard]] constexpr float margin_after() const noexcept
    {
        return empty() ? 0 : _forward ? back().margin_after : front().margin_after;
    }

    [[nodiscard]] constexpr float padding_before() const noexcept
    {
        return empty() ? 0 : _forward ? front().padding_before : back().padding_before;
    }

    [[nodiscard]] constexpr float padding_after() const noexcept
    {
        return empty() ? 0 : _forward ? back().padding_after : front().padding_after;
    }

    [[nodiscard]] constexpr std::tuple<float, float, float> update_constraints() const noexcept
    {
        return constraints(begin(), end());
    }

    /** Get the minimum, preferred, maximum size of the span.
     *
     * The returned minimum, preferred and maximum include the internal margin within the span.
     *
     * @param cell The reference to the cell in the grid.
     * @return The minimum, preferred and maximum size.
     */
    [[nodiscard]] constexpr std::tuple<float, float, float> constraints(cell_type const& cell) const noexcept
    {
        return constraints(cell.template first<axis>(), cell.template last<axis>());
    }

    [[nodiscard]] constexpr float position(cell_type const& cell) const noexcept
    {
        return position(cell.template first<axis>(), cell.template last<axis>());
    }

    [[nodiscard]] constexpr float extent(cell_type const& cell) const noexcept
    {
        return extent(cell.template first<axis>(), cell.template last<axis>());
    }

    [[nodiscard]] constexpr std::optional<float> guideline(cell_type const& cell) const noexcept
    {
        if (cell.template span<axis>() == 1) {
            return guideline(cell.template first<axis>());
        } else {
            return std::nullopt;
        }
    }

    /** Layout each cell along an axis.
     *
     * The algorithm works as follows:
     *  1. Initialize each cell based on its preferred size.
     *  2. While the grid needs to be shrunk:
     *    a. Calculate the amount of cells that are allowed to shrink.
     *    b. Apply shrinkage to the cells that are allowed to, up to the minimum.
     *    c. If all the cells are maximum shrunk, stop.
     *  3. While the grid needs to be expanded:
     *    a. Calculate the amount of cells that are allowed to expand.
     *    b. Apply expansion to the cells that are allowed to, up to the maximum.
     *    c. If all the cells are maximum expanded, goto 4.
     *  4. Expand the largest cell to make it fit.
     *
     * In an emergency widgets will get a size larger than its maximum. However
     * widgets will never get a smaller size than its minimum.
     *
     * @param new_position The start of the grid along its axis.
     * @param new_extent The size of the grid along its axis.
     * @param external_guideline The position of the guideline external from the grid.
     * @param guideline_width The width of the guideline.
     */
    constexpr void layout(float new_position, float new_extent, std::optional<float> external_guideline, float guideline_width) noexcept
    {
        // Start with the extent of each constraint equal to the preferred extent.
        for (auto& constraint : _constraints) {
            constraint.extent = constraint.preferred;
        }

        // If the total extent is too large, shrink the constraints that allow to be shrunk.
        auto [total_extent, count] = layout_shrink(begin(), end());
        while (total_extent > new_extent and count != 0) {
            // The result may shrink slightly too much, which will be fixed by expanding in the next loop.
            std::tie(total_extent, count) = layout_shrink(begin(), end(), total_extent - new_extent, count);
        }

        // If the total extent is too small, expand the constraints that allow to be grown.
        std::tie(total_extent, count) = layout_expand(begin(), end());
        while (total_extent < new_extent and count != 0) {
            // The result may expand slightly too much, we don't care.
            std::tie(total_extent, count) = layout_expand(begin(), end(), new_extent - total_extent, count);
        }

        // If the total extent is still too small, expand into the cells that are marked beyond_maximum.
        if (total_extent < new_extent) {
            // The result may expand slightly too much, we don't care.
            count = std::count_if(begin(), end(), [](hilet& item) {
                return item.beyond_maximum;
            });
            if (count) {
                auto expand = new_extent - total_extent;
                hilet expand_per = std::ceil(expand / count);

                for (auto& constraint : _constraints) {
                    hilet expand_this = std::min(expand_per, expand);
                    if (constraint.beyond_maximum) {
                        constraint.extent += expand_this;
                        expand -= expand_this;
                    }
                }
            }
            total_extent = extent(cbegin(), cend());
        }

        // If the total extent is still too small, expand the first constrain above the maximum size.
        if (total_extent < new_extent and not empty()) {
            // The result may expand slightly too much, we don't care.
            front().extent += new_extent - total_extent;
        }

        if (_forward) {
            layout_position(begin(), end(), new_position, guideline_width);
        } else {
            layout_position(rbegin(), rend(), new_position, guideline_width);
        }

        if (external_guideline and size() == 1) {
            // When there is only 1 cell on this axis, the external guideline is used.
            // XXX If there are more cell, then the external alignment should be taken into account.
            front().guideline = *external_guideline;
        }
    }

    /** Number of cell on this axis.
     */
    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return _constraints.size();
    }

    /** Check if this axis is empty.
     */
    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _constraints.empty();
    }

    /** Iterator to the first cell on this axis.
     */
    [[nodiscard]] constexpr iterator begin() noexcept
    {
        return _constraints.begin();
    }

    /** Iterator to the first cell on this axis.
     */
    [[nodiscard]] constexpr const_iterator begin() const noexcept
    {
        return _constraints.begin();
    }

    /** Iterator to the first cell on this axis.
     */
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return _constraints.cbegin();
    }

    /** Iterator to beyond the last cell on this axis.
     */
    [[nodiscard]] constexpr iterator end() noexcept
    {
        return _constraints.end();
    }

    /** Iterator to beyond the last cell on this axis.
     */
    [[nodiscard]] constexpr const_iterator end() const noexcept
    {
        return _constraints.end();
    }

    /** Iterator to beyond the last cell on this axis.
     */
    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return _constraints.cend();
    }

    /** Iterator to the first cell on this axis.
     */
    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept
    {
        return _constraints.rbegin();
    }

    /** Iterator to the first cell on this axis.
     */
    [[nodiscard]] constexpr reverse_iterator rend() noexcept
    {
        return _constraints.rend();
    }

    /** Get element.
     *
     * @note It is undefined behavior to index beyond the number of cell on this axis.
     * @param index The index of the cell.
     * @return A reference to the cell.
     */
    [[nodiscard]] constexpr reference operator[](size_t index) noexcept
    {
        hi_axiom(index < size());
        return _constraints[index];
    }

    /** Get element.
     *
     * @note It is undefined behavior to index beyond the number of cell on this axis.
     * @param index The index of the cell.
     * @return A reference to the cell.
     */
    [[nodiscard]] constexpr const_reference operator[](size_t index) const noexcept
    {
        hi_axiom(index < size());
        return _constraints[index];
    }

    /** Get the first element.
     *
     * @note It is undefined behavior to call this function on an empty axis.
     * @return A reference to the first cell.
     */
    [[nodiscard]] constexpr reference front() noexcept
    {
        hi_axiom(not empty());
        return _constraints.front();
    }

    /** Get the first element.
     *
     * @note It is undefined behavior to call this function on an empty axis.
     * @return A reference to the first cell.
     */
    [[nodiscard]] constexpr const_reference front() const noexcept
    {
        hi_axiom(not empty());
        return _constraints.front();
    }

    /** Get the last element.
     *
     * @note It is undefined behavior to call this function on an empty axis.
     * @return A reference to the last cell.
     */
    [[nodiscard]] constexpr reference back() noexcept
    {
        hi_axiom(not empty());
        return _constraints.back();
    }

    /** Get the last element.
     *
     * @note It is undefined behavior to call this function on an empty axis.
     * @return A reference to the last cell.
     */
    [[nodiscard]] constexpr const_reference back() const noexcept
    {
        hi_axiom(not empty());
        return _constraints.back();
    }

private:
    /** The constraints.
     *
     * There is one merged-constraint per cell along the axis;
     * plus one extra constraint with only `margin_before` being valid.
     *
     * Using one extra constraint reduces the amount of if-statements.
     *
     */
    constraint_vector _constraints = {};

    /** The constraints are defined in left-to-right, bottom-to-top order.
     */
    bool _forward = true;

    /** Shrink cells.
     *
     * This function is called in two different ways:
     *  - First without @a extra and @a count to get the number of pixels of the cells and the number
     *    of cells that shrink further. These values are used to calculate @a extra and @a count
     *    of the next iteration.
     *  - Continued with @a extra and @a count filled in.
     *
     * @note Must be called after `layout_initialize()`.
     * @note It is undefined behavior to pass zero in @a count.
     * @param first The iterator to the first cell to shrink.
     * @param last The iterator to beyond the last cell to shrink.
     * @param shrink The total number of pixels to shrink spread over the cells
     * @param count The number of cells between first/last that can be shrunk, from previous iteration.
     * @return Number of pixels of the cells and inner-margins, number of cells in the range that can shrink more.
     */
    [[nodiscard]] constexpr std::pair<float, size_t>
    layout_shrink(const_iterator first, const_iterator last, float shrink = 0.0f, size_t count = 1) noexcept
    {
        hilet first_ = begin() + std::distance(cbegin(), first);
        hilet last_ = begin() + std::distance(cbegin(), last);

        hi_axiom(shrink >= 0);

        hilet shrink_per = std::floor(shrink / count);

        auto new_extent = 0.0f;
        auto new_count = 0_uz;
        for (auto it = first_; it != last_; ++it) {
            hilet shrink_this = std::max({shrink_per, shrink, it->extent - it->minimum});
            it->extent -= shrink_this;
            shrink -= shrink_this;

            if (it != first_) {
                new_extent += it->margin_before;
            }
            new_extent += it->extent;

            if (it->extent > it->minimum) {
                ++new_count;
            }
        }

        return {new_extent, new_count};
    }

    /** Expand cells.
     *
     * This function is called in two different ways:
     *  - First without @a extra and @a count to get the number of pixels of the cells and the number
     *    of cells that expand further. These values are used to calculate @a extra and @a count
     *    of the next iteration.
     *  - Continued with @a extra and @a count filled in.
     *
     * @note Must be called after `layout_initialize()`.
     * @note It is undefined behavior to pass zero in @a count.
     * @param first The iterator to the first cell to expand.
     * @param last The iterator to beyond the last cell to expand.
     * @param expand The total number of pixels to expand spread over the cells
     * @param count The number of cells between first/last that can be expanded, from previous iteration.
     * @return Number of pixels of the cells and inner-margins, number of cells in the range that can expand more.
     */
    [[nodiscard]] constexpr std::pair<float, size_t>
    layout_expand(const_iterator first, const_iterator last, float expand = 0.0f, size_t count = 1) noexcept
    {
        hilet first_ = begin() + std::distance(cbegin(), first);
        hilet last_ = begin() + std::distance(cbegin(), last);

        hi_axiom(expand >= 0.0f);

        hilet expand_per = std::ceil(expand / count);
        hi_axiom(expand_per >= 0.0f);

        auto new_extent = 0.0f;
        auto new_count = 0_uz;
        for (auto it = first_; it != last_; ++it) {
            hilet expand_this = std::min({expand_per, expand, it->maximum - it->extent});
            it->extent += expand_this;
            expand -= expand_this;

            if (it != first_) {
                new_extent += it->margin_before;
            }
            new_extent += it->extent;

            if (it->extent < it->maximum) {
                ++new_count;
            }
        }

        return {new_extent, new_count};
    }

    constexpr void layout_position(auto first, auto last, float start_position, float guideline_width) noexcept
    {
        auto position = start_position;
        for (auto it = first; it != last; ++it) {
            it->position = position;
            it->guideline = make_guideline(
                it->alignment, position, position + it->extent, it->padding_before, it->padding_after, guideline_width);

            position += it->extent;
            position += it->margin_after;
        }
    }

    /** Construct from a simple cell.
     *
     * Calculate all the margins. And the minimum, preferred and maximum size
     * for a cell that has a span of one in the direction of the axis.
     *
     * @param cell The cell to construct.
     */
    constexpr void construct_simple_cell(cell_type const& cell) noexcept
    {
        inplace_max(_constraints[cell.template first<axis>()].margin_before, cell.template margin_before<axis>(_forward));
        inplace_max(_constraints[cell.template last<axis>() - 1].margin_after, cell.template margin_after<axis>(_forward));
        inplace_max(_constraints[cell.template first<axis>()].padding_before, cell.template padding_before<axis>(_forward));
        inplace_max(_constraints[cell.template last<axis>() - 1].padding_after, cell.template padding_after<axis>(_forward));

        for (auto i = cell.template first<axis>(); i != cell.template last<axis>(); ++i) {
            _constraints[i].beyond_maximum |= cell.beyond_maximum;
        }

        if (cell.template span<axis>() == 1) {
            inplace_max(_constraints[cell.template first<axis>()].alignment, cell.template alignment<axis>());
            inplace_max(_constraints[cell.template first<axis>()].minimum, cell.template minimum<axis>());
            inplace_max(_constraints[cell.template first<axis>()].preferred, cell.template preferred<axis>());
            inplace_min(_constraints[cell.template first<axis>()].maximum, cell.template maximum<axis>());
        }
    }

    /** Construct from a span-cell.
     *
     * Spread the size of a multi-span.
     *
     * @param cell The cell to construct.
     */
    constexpr void construct_span_cell(cell_type const& cell) noexcept
    {
        auto num_cells = narrow_cast<float>(cell.template span<axis>());

        if (cell.template span<axis>() > 1) {
            hilet[span_minimum, span_preferred, span_maximum] = constraints(cell);
            if (hilet extra = cell.template minimum<axis>() - span_minimum; extra > 0) {
                hilet extra_per_cell = std::floor(extra / num_cells);
                for (auto i = cell.template first<axis>(); i != cell.template last<axis>(); ++i) {
                    _constraints[i].minimum += extra_per_cell;
                }
            }

            if (hilet extra = cell.template preferred<axis>() - span_preferred; extra > 0) {
                hilet extra_per_cell = std::floor(extra / num_cells);
                for (auto i = cell.template first<axis>(); i != cell.template last<axis>(); ++i) {
                    _constraints[i].preferred += extra_per_cell;
                }
            }

            if (hilet extra = cell.template maximum<axis>() - span_preferred; extra < 0) {
                hilet extra_per_cell = std::ceil(extra / num_cells);
                for (auto i = cell.template first<axis>(); i != cell.template last<axis>(); ++i) {
                    // The maximum could become too low here, fixup() will fix this.
                    _constraints[i].maximum += extra_per_cell;
                }
            }
        }
    }

    /** Construct fix-up.
     *
     * Fix-up minimum, preferred, maximum. And calculate the padding.
     */
    constexpr void construct_fixup() noexcept
    {
        for (auto it = begin(); it != end(); ++it) {
            // Fix the margins so that between two constraints they are equal.
            if (it + 1 != end()) {
                it->margin_after = (it + 1)->margin_before = std::max(it->margin_after, (it + 1)->margin_before);
            }

            // Fix the constraints so that minimum <= preferred <= maximum.
            inplace_max(it->preferred, it->minimum);
            inplace_max(it->maximum, it->preferred);

            // Fix the padding, so that it doesn't overlap.
            if (it->padding_before + it->padding_after > it->minimum) {
                hilet padding_diff = it->padding_after - it->padding_before;
                hilet middle = std::clamp(it->minimum / 2.0f + padding_diff, 0.0f, it->minimum);
                it->padding_after = middle;
                it->padding_before = it->minimum - middle;
            }
        }
    }

    /** Get the minimum, preferred, maximum size of the span.
     *
     * The returned minimum, preferred and maximum include the internal margin within the span.
     *
     * @param first The iterator to the first cell.
     * @param last The iterator beyond the last cell.
     * @return The minimum, preferred and maximum size.
     */
    [[nodiscard]] constexpr std::tuple<float, float, float> constraints(const_iterator first, const_iterator last) const noexcept
    {
        auto r_minimum = 0.0f;
        auto r_preferred = 0.0f;
        auto r_maximum = 0.0f;
        auto r_margin = 0.0f;

        if (first != last) {
            r_minimum = first->minimum;
            r_preferred = first->preferred;
            r_maximum = first->maximum;
            for (auto it = first + 1; it != last; ++it) {
                r_margin += it->margin_before;
                r_minimum += it->minimum;
                r_preferred += it->preferred;
                r_maximum += it->maximum;
            }
        }
        return {r_minimum + r_margin, r_preferred + r_margin, r_maximum + r_margin};
    }

    /** Get the minimum, preferred, maximum size of the span.
     *
     * The returned minimum, preferred and maximum include the internal margin within the span.
     *
     * @param first The index to the first cell.
     * @param last The index beyond the last cell.
     * @return The minimum, preferred and maximum size.
     */
    [[nodiscard]] constexpr std::tuple<float, float, float> constraints(size_t first, size_t last) const noexcept
    {
        hi_axiom(first <= last);
        hi_axiom(last <= size());
        return constraints(begin() + first, begin() + last);
    }

    /** Get the current layout position of a span.
     *
     * @note valid after layout.
     * @param first The iterator to the first cell.
     * @param last The iterator beyond the last cell.
     * @return The current size of the span, including internal margins.
     */
    [[nodiscard]] constexpr float position(const_iterator first, const_iterator last) const noexcept
    {
        hi_axiom(first != last);
        if (_forward) {
            return first->position;
        } else {
            return (last - 1)->position;
        }
    }

    /** Get the current layout position of a span.
     *
     * @note valid after layout.
     * @param first The index to the first cell.
     * @param last The index beyond the last cell.
     * @return The current size of the span, including internal margins.
     */
    [[nodiscard]] constexpr float position(size_t first, size_t last) const noexcept
    {
        hi_axiom(first < last);
        hi_axiom(last <= size());
        return position(cbegin() + first, cbegin() + last);
    }

    /** Get the current layout size of a span.
     *
     * @note valid after layout.
     * @param first The iterator to the first cell.
     * @param last The iterator beyond the last cell.
     * @return The current size of the span, including internal margins.
     */
    [[nodiscard]] constexpr float extent(const_iterator first, const_iterator last) const noexcept
    {
        auto r = 0.0f;
        if (first != last) {
            r = first->extent;
            for (auto it = first + 1; it != last; ++it) {
                r += it->margin_before;
                r += it->extent;
            }
        }
        return r;
    }

    /** Get the current layout size of a span.
     *
     * @note valid after layout.
     * @param first The index to the first cell.
     * @param last The index beyond the last cell.
     * @return The current size of the span, including internal margins.
     */
    [[nodiscard]] constexpr float extent(size_t first, size_t last) const noexcept
    {
        hi_axiom(first <= last);
        hi_axiom(last <= size());
        return extent(cbegin() + first, cbegin() + last);
    }

    [[nodiscard]] constexpr std::optional<float> guideline(const_iterator it) const noexcept
    {
        return it->guideline;
    }

    [[nodiscard]] constexpr std::optional<float> guideline(size_t i) const noexcept
    {
        return guideline(cbegin() + i);
    }
};

} // namespace detail

/** Grid layout algorithm.
 *
 *
 *
 */
template<typename T>
class grid_layout {
public:
    using value_type = T;

    using cell_type = detail::grid_layout_cell<value_type>;
    using cell_vector = std::vector<cell_type>;
    using iterator = cell_vector::iterator;
    using const_iterator = cell_vector::const_iterator;
    using reference = cell_vector::reference;
    using const_reference = cell_vector::const_reference;

    ~grid_layout() = default;
    constexpr grid_layout() noexcept = default;
    constexpr grid_layout(grid_layout const&) noexcept = default;
    constexpr grid_layout(grid_layout&&) noexcept = default;
    constexpr grid_layout& operator=(grid_layout const&) noexcept = default;
    constexpr grid_layout& operator=(grid_layout&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(grid_layout const&, grid_layout const&) noexcept = default;

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _cells.empty();
    }

    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return _cells.size();
    }

    [[nodiscard]] constexpr size_t num_columns() const noexcept
    {
        return _num_columns;
    }

    [[nodiscard]] constexpr size_t num_rows() const noexcept
    {
        return _num_rows;
    }

    [[nodiscard]] constexpr iterator begin() noexcept
    {
        return _cells.begin();
    }

    [[nodiscard]] constexpr iterator end() noexcept
    {
        return _cells.end();
    }

    [[nodiscard]] constexpr const_iterator begin() const noexcept
    {
        return _cells.begin();
    }

    [[nodiscard]] constexpr const_iterator end() const noexcept
    {
        return _cells.end();
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return _cells.cbegin();
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return _cells.cend();
    }

    [[nodiscard]] constexpr const_reference operator[](size_t i) const noexcept
    {
        return _cells[i];
    }

    [[nodiscard]] constexpr reference operator[](size_t i) noexcept
    {
        return _cells[i];
    }

    /** Check if the cell on the grid is already in use.
     *
     * @param first_column The first column of the cell-span.
     * @param first_row The first row of the cell-span.
     * @param last_column One beyond the last column of the cell-span.
     * @param last_row One beyond the last row of the cell-span.
     * @retval true If the given cell-span overlaps with an already existing cell.
     */
    [[nodiscard]] constexpr bool cell_in_use(size_t first_column, size_t first_row, size_t last_column, size_t last_row) noexcept
    {
        // At least one cell must be in the range.
        hi_axiom(first_column < last_column);
        hi_axiom(first_row < last_row);

        for (hilet& cell : _cells) {
            if (first_column >= cell.last_column) {
                continue;
            }
            if (last_column <= cell.first_column) {
                continue;
            }
            if (first_row >= cell.last_row) {
                continue;
            }
            if (last_row <= cell.first_row) {
                continue;
            }
            return true;
        }
        return false;
    }

    /** Check if the cell on the grid is already in use.
     *
     * @param first_column The first column of the cell-span.
     * @param last_column One beyond the last column of the cell-span.
     * @param first_row The first row of the cell-span.
     * @param last_row One beyond the last row of the cell-span.
     * @param value The value to be copied or moved into the cell.
     * @param beyond_maximum Allow this cell to resize beyond the maximum constraint.
     * @return A reference to the created cell.
     */
    template<forward_of<value_type> Value>
    constexpr reference add_cell(
        size_t first_column,
        size_t first_row,
        size_t last_column,
        size_t last_row,
        Value&& value,
        bool beyond_maximum = false) noexcept
    {
        // At least one cell must be in the range.
        hi_assert(first_column < last_column);
        hi_assert(first_row < last_row);
        hi_assert(not cell_in_use(first_column, first_row, last_column, last_row));
        auto& r = _cells.emplace_back(first_column, first_row, last_column, last_row, beyond_maximum, std::forward<Value>(value));
        update_after_insert_or_delete();
        return r;
    }

    /** Check if the cell on the grid is already in use.
     *
     * @param column The column of the cell.
     * @param row The row of the cell.
     * @param value The value to be copied or moved into the cell.
     * @param beyond_maximum Allow this cell to resize beyond the maximum constraint.
     * @return A reference to the created cell.
     */
    template<forward_of<value_type> Value>
    constexpr reference add_cell(size_t column, size_t row, Value&& value, bool beyond_maximum = false) noexcept
    {
        return add_cell(column, row, column + 1, row + 1, std::forward<Value>(value), beyond_maximum);
    }

    constexpr void clear() noexcept
    {
        _cells.clear();
        update_after_insert_or_delete();
    }

    [[nodiscard]] constexpr box_constraints constraints(bool left_to_right) const noexcept
    {
        // Rows in the grid are laid out from top to bottom which is reverse from the y-axis up.
        _row_constraints = {_cells, num_rows(), false};
        _column_constraints = {_cells, num_columns(), left_to_right};

        auto r = box_constraints{};
        std::tie(r.minimum.width(), r.preferred.width(), r.maximum.width()) = _column_constraints.update_constraints();
        r.margins.left() = _column_constraints.margin_before();
        r.margins.right() = _column_constraints.margin_after();
        r.padding.left() = _column_constraints.padding_before();
        r.padding.right() = _column_constraints.padding_after();

        std::tie(r.minimum.height(), r.preferred.height(), r.maximum.height()) = _row_constraints.update_constraints();
        r.margins.bottom() = _row_constraints.margin_after();
        r.margins.top() = _row_constraints.margin_before();
        r.padding.bottom() = _row_constraints.padding_after();
        r.padding.top() = _row_constraints.padding_before();

        r.alignment = [&] {
            if (num_rows() == 1 and num_columns() == 1) {
                return hi::alignment{_column_constraints.front().alignment, _row_constraints.front().alignment};
            } else if (num_rows() == 1) {
                return hi::alignment{_row_constraints.front().alignment};
            } else if (num_columns() == 1) {
                return hi::alignment{_column_constraints.front().alignment};
            } else {
                return hi::alignment{};
            }
        }();

        return r;
    }

    /** Layout the cells based on the width and height.
     *
     * @param shape The shape of the box to place the grid in.
     * @param baseline_adjustment How much the baseline needs to be adjusted when aligned to the top.
     */
    constexpr void set_layout(box_shape const& shape, float baseline_adjustment) noexcept
    {
        // Rows in the grid are laid out from top to bottom which is reverse from the y-axis up.
        _column_constraints.layout(shape.x(), shape.width(), shape.centerline, 0);
        _row_constraints.layout(shape.y(), shape.height(), shape.baseline, baseline_adjustment);

        // Assign the shape for each cell.
        for (auto& cell : _cells) {
            cell.shape.rectangle = {
                _column_constraints.position(cell),
                _row_constraints.position(cell),
                _column_constraints.extent(cell),
                _row_constraints.extent(cell)};
            cell.shape.centerline = _column_constraints.guideline(cell);
            cell.shape.baseline = _row_constraints.guideline(cell);
        }
    }

private:
    cell_vector _cells = {};
    size_t _num_rows = 0;
    size_t _num_columns = 0;
    mutable detail::grid_layout_axis_constraints<axis::y, value_type> _row_constraints = {};
    mutable detail::grid_layout_axis_constraints<axis::x, value_type> _column_constraints = {};

    /** Sort the cells ordered by row then column.
     *
     * The ordering is the same as they keyboard focus chain order.
     */
    constexpr void sort_cells() noexcept
    {
        std::sort(_cells.begin(), _cells.end(), [](cell_type const& lhs, cell_type const& rhs) {
            if (lhs.first_row != rhs.first_row) {
                return lhs.first_row < rhs.first_row;
            } else {
                return lhs.first_column < rhs.first_column;
            }
        });
    }

    /** Updates needed after a cell was added or removed.
     */
    constexpr void update_after_insert_or_delete() noexcept
    {
        sort_cells();

        _num_rows = 0;
        _num_columns = 0;
        for (hilet& cell : _cells) {
            inplace_max(_num_rows, cell.last_row);
            inplace_max(_num_columns, cell.last_column);
        }
    }
};

}} // namespace hi::v1
