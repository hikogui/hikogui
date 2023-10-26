// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <algorithm>
#include <tuple>
#include <cmath>
#include <iterator>
#include <bit>
#include <span>
#include <vector>
#include <cstddef>

export module hikogui_algorithm_algorithm_misc;
import hikogui_utility;

namespace hi::inline v1 {

/** Transform an input container to the output container.
 * @param input Input container.
 * @param operation A function to execute on each element in the input
 * @return Output container containing the transformed elements.
 */
template<typename T, typename U, typename F>
inline T transform(const U& input, F operation)
{
    T result = {};
    result.reserve(input.size());
    std::transform(input.begin(), input.end(), std::back_inserter(result), operation);
    return result;
}

/** Generate data in an array.
 * @param operation A function to execute for each element in the array.
 *                  The function accepts a single index argument.
 * @return An array filled with elements generated by operation.
 */
template<typename T, std::size_t N, typename F>
constexpr std::array<T, N> generate_array(F operation)
{
    std::array<T, N> a{};

    for (std::size_t i = 0; i < N; i++) {
        a.at(i) = operation(i);
    }

    return a;
}

/** Remove element from a container.
 *
 * @note The elements before @a element remain in order.
 * @note The elements after @a element are not in the original order.
 * @param first The iterator pointing to the first element of the container
 * @param last The iterator pointing one beyond the last element of the container.
 * @param element The iterator that points to the element to be removed.
 * @return The iterator one past the last element.
 */
template<typename It>
constexpr It unordered_remove(It first, It last, It element)
{
    hi_axiom(first != last);

    using std::swap;

    auto new_last = last - 1;
    swap(*element, *new_last);
    return new_last;
}

template<typename It, typename UnaryPredicate>
constexpr It rfind_if(It const first, It const last, UnaryPredicate predicate)
{
    auto i = last;
    do {
        i--;
        if (predicate(*i)) {
            return i;
        }
    } while (i != first);
    return last;
}

template<typename It, typename UnaryPredicate>
constexpr It rfind_if_not(It const first, It const last, UnaryPredicate predicate)
{
    return rfind_if(first, last, [&](hilet& x) {
        return !predicate(x);
    });
}

template<typename It, typename T>
constexpr It rfind(It const first, It const last, T const& value)
{
    return rfind_if(first, last, [&](hilet& x) {
        return x == value;
    });
}

/** Find the first occurrence of an value in a data.
 * @param data_first An iterator pointing to the first item of data.
 * @param data_last An iterator pointing one beyond the last item of data.
 * @param value_first An iterator pointing to a value to find in data.
 * @param value_last An iterator pointing on beyond the last value to find in data.
 * @return An iterator within data for the first matching value, or data_last if not found.
 */
template<typename It, typename ItAny>
[[nodiscard]] constexpr It find_any(It data_first, It data_last, ItAny value_first, ItAny value_last) noexcept
{
    return std::find_if(data_first, data_last, [value_first, value_last](hilet& data) {
        return std::any_of(value_first, value_last, [&data](hilet& value) {
            return data == value;
        });
    });
}

/** Find the start of the current cluster.
 * @param last The last iterator, where this function will stop iterating.
 * @param start Where to start the search
 * @param predicate A function returning the identifier of the cluster.
 * @return One beyond the last iterator where the cluster is the same as start.
 */
template<typename ConstIt, typename It, typename UnaryPredicate>
constexpr It find_cluster(ConstIt last, It start, UnaryPredicate predicate)
{
    hilet cluster_id = predicate(*start);

    for (auto i = start + 1; i != last; ++i) {
        if (predicate(*i) != cluster_id) {
            return i;
        }
    }
    return last;
}

/** Find the start of the current cluster.
 * @param first The first iterator, where this function will stop iterating.
 * @param start Where to start the search
 * @param predicate A function returning the identifier of the cluster.
 * @return The first iterator where the cluster is the same as start.
 */
template<typename ConstIt, typename It, typename UnaryPredicate>
constexpr It rfind_cluster(ConstIt first, It start, UnaryPredicate predicate)
{
    hilet cluster_id = predicate(*start);

    if (start == first) {
        return first;
    }

    auto i = start - 1;
    while (true) {
        if (predicate(*i) != cluster_id) {
            return (i + 1);
        }

        if (i == first) {
            return i;
        }
        --i;
    }
    std::unreachable();
}

/** Find the begin and end of the current cluster.
 * @param first The first iterator, where this function will stop iterating.
 * @param last The last iterator, where this function will stop iterating.
 * @param start Where to start the search
 * @param predicate A function returning the identifier of the cluster.
 * @return The first and one beyond last iterator where the cluster is the same as start.
 */
template<typename ConstIt, typename It, typename UnaryPredicate>
constexpr std::pair<It, It> bifind_cluster(ConstIt first, ConstIt last, It start, UnaryPredicate predicate)
{
    return {rfind_cluster(first, start, predicate), find_cluster(last, start, predicate)};
}

/*! For each cluster.
 * func() is executed for each cluster that is found between first-last.
 * A cluster is found between two separators, a separator is detected with IsClusterSeperator().
 * A cluster does not include the separator itself.
 */
template<typename It, typename S, typename F>
inline void for_each_cluster(It first, It last, S IsClusterSeperator, F Function)
{
    if (first == last) {
        return;
    }

    // If the first item is a cluster separator skip over it.
    if (IsClusterSeperator(*first)) {
        first++;
    }

    for (auto i = first; i != last;) {
        auto j = std::find_if(i, last, IsClusterSeperator);
        Function(i, j);

        auto skipOverSeperator = (j == last) ? 0 : 1;
        i = j + skipOverSeperator;
    }
}

template<typename InputIt1, typename InputIt2, typename BinaryPredicate>
inline std::pair<InputIt1, InputIt2>
rmismatch(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, BinaryPredicate predicate) noexcept
{
    auto i1 = last1;
    auto i2 = last2;

    while (true) {
        if (i1 == first1 && i2 == first2) {
            return {last1, last2};
        } else if (i1 == first1) {
            return {last1, --i2};
        } else if (i2 == first2) {
            return {--i1, last2};
        }

        if (!predicate(*(--i1), *(--i2))) {
            return {i1, i2};
        }
    }
}

template<typename InputIt1, typename InputIt2>
inline std::pair<InputIt1, InputIt2> rmismatch(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2) noexcept
{
    return rmismatch(first1, last1, first2, last2, [&](auto a, auto b) {
        return a == b;
    });
}

template<typename T>
T smoothstep(T x) noexcept
{
    x = std::clamp(x, T{0.0}, T{1.0});
    return x * x * (3 - 2 * x);
}

template<typename T>
T inverse_smoothstep(T x)
{
    return T{0.5} - std::sin(std::asin(T{1.0} - T{2.0} * x) / T{3.0});
}

/** Shuffle a container based on a list of indices.
 * It is undefined behavior for an index to point beyond `last`.
 * It is undefined behavior for an index to repeat.
 *
 * Complexity is O(n) swaps, where n is the number of indices.
 *
 * @param first An iterator pointing to the first item in a container to be shuffled (index = 0)
 * @param last An iterator pointing beyond the last item in a container to be shuffled.
 * @param indices_first An iterator pointing to the first index.
 * @param indices_last An iterator pointing beyond the last index.
 * @param index_op A function returning the `size` index from indices.
 *                 The default returns the index item it self.
 * @return An iterator pointing beyond the last element that was added by the indices.
 *         first + std::distance(indices_first, indices_last)
 */
auto shuffle_by_index(auto first, auto last, auto indices_first, auto indices_last, auto index_op) noexcept
{
    std::size_t src_size = std::distance(first, last);

    // Keep track of index locations during shuffling of items.
    auto src_indices = std::vector<std::size_t>{};
    src_indices.reserve(src_size);
    for (std::size_t i = 0; i != src_size; ++i) {
        src_indices.push_back(i);
    }

    std::size_t dst = 0;
    for (auto it = indices_first; it != indices_last; ++it, ++dst) {
        hilet index = index_op(*it);
        hi_assert_bounds(index, src_indices);

        auto src = [&src_indices, index]() {
            auto src = index;
            do {
                src = src_indices[src];
            } while (src_indices[src] != index);
            return src;
        }();

        if (src != dst) {
            std::iter_swap(first + src, first + dst);
            std::iter_swap(begin(src_indices) + src, begin(src_indices) + dst);
        }
    }

    return first + dst;
}

/** Shuffle a container based on a list of indices.
 * It is undefined behavior for an index to point beyond `last`.
 * It is undefined behavior for an index to repeat.
 *
 * Complexity is O(n) swaps, where n is the number of indices.
 *
 * @param first An iterator pointing to the first item in a container to be shuffled (index = 0)
 * @param last An iterator pointing beyond the last item in a container to be shuffled.
 * @param indices_first An iterator pointing to the first index.
 * @param indices_last An iterator pointing beyond the last index.
 * @return An iterator pointing beyond the last element that was added by the indices.
 *         first + std::distance(indices_first, indices_last)
 */
auto shuffle_by_index(auto first, auto last, auto indices_first, auto indices_last) noexcept
{
    return shuffle_by_index(first, last, indices_first, indices_last, [](hilet& x) {
        return narrow_cast<std::size_t>(x);
    });
}

/** Strip data from the front side.
 * @param data_first The iterator pointing to the first element of data.
 * @param data_last The iterator pointing one beyond the last element of data.
 * @param value_first The iterator pointing to the first value to be removed from data.
 * @param value_last The iterator pointing one beyond the last value to be removed from data.
 * @return An iterator pointing to the first data element not belonging to the values to be stripped.
 *         or data_last when all data elements have been stripped.
 */
template<typename DataIt, typename ValueIt>
DataIt front_strip(DataIt data_first, DataIt data_last, ValueIt value_first, ValueIt value_last) noexcept
{
    for (auto it = data_first; it != data_last; ++it) {
        if (std::find(value_first, value_last, *it) == value_last) {
            // Return the iterator pointing to the data that is not part of the value set.
            return it;
        }
    }

    return data_last;
}

/** Strip data from the back side.
 * @param data_first The iterator pointing to the first element of data.
 * @param data_last The iterator pointing one beyond the last element of data.
 * @param value_first The iterator pointing to the first value to be removed from data.
 * @param value_last The iterator pointing one beyond the last value to be removed from data.
 * @return An iterator pointing one beyond the first data element not belonging to the values to be stripped.
 *         or data_first when all data elements have been stripped.
 */
template<typename DataIt, typename ValueIt>
DataIt back_strip(DataIt data_first, DataIt data_last, ValueIt value_first, ValueIt value_last) noexcept
{
    auto it = data_last;
    while (it != data_first) {
        if (std::find(value_first, value_last, *(--it)) == value_last) {
            // Return an iterator one beyond the data that is not part of the value set.
            return it + 1;
        }
    }

    return data_first;
}

/** The fast lower bound algorithm.
 *
 * @tparam T The type of the table element.
 * @tparam Key The type of the key
 * @tparam Endian The endianess of the key in the table.
 * @param table A span of elements to search, the key is in the first bytes of each element.
 * @param key A unsigned integral to search in the elements.
 * @return A pointer to the entry found,or nullptr if not found.
 */
template<std::endian Endian = std::endian::native, typename T, std::unsigned_integral Key>
[[nodiscard]] constexpr T *fast_lower_bound(std::span<T> table, Key const& key) noexcept
{
    auto base = table.data();
    auto len = table.size();

    // A faster lower-bound search with less branches that are more predictable.
    while (len > 1) {
        hi_axiom_not_null(base);

        hilet half = len / 2;

        if (load<Key, Endian>(base + half - 1) < key) {
            base += half;
        }
        len -= half;
    }
    if (load<Key, Endian>(base) < key) {
        return nullptr;
    }
    return base;
}

/** Search for the item that is equal to the key.
 */
template<std::endian Endian = std::endian::native, typename T, std::unsigned_integral Key>
[[nodiscard]] constexpr T *fast_binary_search_eq(std::span<T> table, Key const& key) noexcept
{
    hilet ptr = fast_lower_bound<Endian>(table, key);
    if (ptr == nullptr) {
        return nullptr;
    }

    if (load<Key, Endian>(ptr) != key) {
        return nullptr;
    }

    return ptr;
}

} // namespace hi::inline v1
