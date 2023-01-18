// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility/module.hpp"
#include <map>

namespace hi::inline v1 {

/** A tree container.
 *
 * @tparam Key They key type to index into each level of the tree.
 * @tparam T The value that is stored into each node.
 * @tparam Compare The comparison to use with the key used in `std::map`.
 */
template<typename Key, typename T, typename Compare = std::less<Key>>
class tree {
public:
    using key_type = Key;
    using value_type = T;

    constexpr ~tree() = default;
    constexpr tree(tree const&) noexcept = default;
    constexpr tree(tree&&) noexcept = default;
    constexpr tree& operator=(tree const&) noexcept = default;
    constexpr tree& operator=(tree&&) noexcept = default;
    constexpr tree() noexcept = default;

    /** Find or create the node and return the value of the node.
     *
     * XXX c++23 multiple argument index operator
     * @param path_first The iterator to the first element of the path of the node to get the value of.
     * @param path_last The iterator beyond the last element of the path of the node to get the value of.
     * @return A reference to the value.
     */
    value_type& operator()(auto path_first, auto path_last) noexcept
    {
        auto *ptr = find_or_create(path_first, path_last);
        hi_assert_not_null(ptr);
        return ptr->value;
    }

    /** Find the node and return the value of the node.
     *
     * XXX c++23 multiple argument index operator
     * @note It is undefined behavior to call this function if the path does not exist in the tree.
     * @param path_first The iterator to the first element of the path of the node to get the value of.
     * @param path_last The iterator beyond the last element of the path of the node to get the value of.
     * @return A reference to the value.
     */
    value_type const& operator()(auto path_first, auto path_last) const noexcept
    {
        auto *ptr = find(path_first, path_last, [](value_type const&) -> void {});
        hi_assert_not_null(ptr);
        return ptr->value;
    }

    /** Find or create the node and return the value of the node.
     *
     * @param key The path of the node to get the value of.
     * @return A reference to the value.
     */
    value_type& operator[](auto const& key) noexcept
    {
        using std::cbegin;
        using std::cend;
        return this->operator()(cbegin(key), cend(key));
    }

    /** Find the node and return the value of the node.
     *
     * @note It is undefined behavior to call this function if the path does not exist in the tree.
     * @param path The path of the node to get the value of.
     * @return A reference to the value.
     */
    value_type const& operator[](auto const& path) const noexcept
    {
        using std::cbegin;
        using std::cend;
        return this->operator()(cbegin(path), cend(path));
    }

    /* Walk the tree from the node pointed to by the path.
     *
     * @param path_first The first element of the path to the start node.
     * @param path_last On beyond the last element of the path to the start node.
     * @param func The function `(value_type &) -> void` to execute on the
     */
    void walk(auto path_first, auto path_last, auto&& func) noexcept
    {
        if (auto element = find(path_first, path_last, [](value_type&) -> void {})) {
            _walk(element, hi_forward(func));
        }
    }

    /* Walk the tree from the node pointed to by the path.
     *
     * @param path_first The first element of the path to the start node.
     * @param path_last On beyond the last element of the path to the start node.
     * @param func The function `(value_type const &) -> void` to execute on the
     */
    void walk(auto path_first, auto path_last, auto&& func) const noexcept
    {
        if (auto element = find(path_first, path_last, [](value_type const&) -> void {})) {
            _walk(element, hi_forward(func));
        }
    }

    /* Walk the tree from the node pointed to by the path.
     *
     * @param path The path (range) to the start node.
     * @param func The function `(value_type &) -> void` to execute on the
     */
    void walk(auto const& key, auto&& func) noexcept
    {
        using std::cbegin;
        using std::cend;
        return walk(cbegin(key), cend(key), hi_forward(func));
    }

    /* Walk the tree from the node pointed to by the path.
     *
     * @param path The path (range) to the start node.
     * @param func The function `(value_type const &) -> void` to execute on the
     */
    void walk(auto const& key, auto&& func) const noexcept
    {
        using std::cbegin;
        using std::cend;
        return walk(cbegin(key), cend(key), hi_forward(func));
    }

    /** Walk the tree starting at path, and also for each node along the path.
     *
     * @param path_first The iterator pointing to the first element of the path; of the node to start walking.
     * @param path_last The iterator pointing beyond the last element of the path.
     * @param func The function `(value_type &) -> void` to call on each child node recursively at the start of
     *             the path, and along the nodes of the path.
     */
    void walk_including_path(auto path_first, auto path_last, auto const& func) noexcept
    {
        if (auto element = find(path_first, path_last, func)) {
            _walk(element, func);
        }
    }

    /** Walk the tree starting at path, and also for each node along the path.
     *
     * @param path_first The iterator pointing to the first element of the path; of the node to start walking.
     * @param path_last The iterator pointing beyond the last element of the path.
     * @param func The function `(value_type const &) -> void` to call on each child node recursively at the start of
     *             the path, and along the nodes of the path.
     */
    void walk_including_path(auto path_first, auto path_last, auto const& func) const noexcept
    {
        if (auto element = find(path_first, path_last, func)) {
            _walk(element, func);
        }
    }

    /** Walk the tree starting at path, and also for each node along the path.
     *
     * @param path The path (range) of the node to start walking.
     * @param func The function `(value_type &) -> void` to call on each child node recursively at the start of
     *             the path, and along the nodes of the path.
     */
    void walk_including_path(auto const& path, auto&& func) noexcept
    {
        using std::cbegin;
        using std::cend;
        return walk_including_path(cbegin(path), cend(path), hi_forward(func));
    }

    /** Walk the tree starting at path, and also for each node along the path.
     *
     * @param path The path (range) of the node to start walking.
     * @param func The function `(value_type const &) -> void` to call on each child node recursively at the start of
     *             the path, and along the nodes of the path.
     */
    void walk_including_path(auto const& path, auto&& func) const noexcept
    {
        using std::cbegin;
        using std::cend;
        return walk_including_path(cbegin(path), cend(path), hi_forward(func));
    }

    /** Walk the full tree.
     *
     * @param func The function `(value_type &) -> void` to call for each node in the tree.
     */
    void walk(auto&& func) noexcept
    {
        _walk(std::addressof(_root), hi_forward(func));
    }

    /** Walk the full tree.
     *
     * @param func The function `(value_type const &) -> void` to call for each node in the tree.
     */
    void walk(auto&& func) const noexcept
    {
        _walk(std::addressof(_root), hi_forward(func));
    }

private:
    struct node_type {
        value_type value;
        std::map<key_type, node_type> children;
    };

    node_type _root;

    /** Find the node at the end of the given path.
     *
     * @param path_first The iterator to the first element of the path.
     * @param path_last The iterator to one beyond the last element of the path.
     * @param func The function `(value_type &) -> void` to call on each node along the path, excluding the node at
     *             at the last element of the path.
     */
    [[nodiscard]] constexpr node_type *find(auto path_first, auto path_last, auto const& func) noexcept
    {
        auto *node = &_root;
        for (auto path_it = path_first; path_it != path_last; ++path_it) {
            func(node->value);

            if (auto node_it = node->children.find(*path_it); node_it != node->children.end()) {
                node = &node_it->second;
            } else {
                return nullptr;
            }
        }
        return node;
    }

    /** Find the node at the end of the given path.
     *
     * @param path_first The iterator to the first element of the path.
     * @param path_last The iterator to one beyond the last element of the path.
     * @param func The function `(value_type const &) -> void` to call on each node along the path, excluding the node at
     *             at the last element of the path.
     */
    [[nodiscard]] constexpr node_type const *find(auto path_first, auto path_last, auto const& func) const noexcept
    {
        node_type const *node = std::addressof(_root);
        for (auto path_it = path_first; path_it != path_last; ++path_it) {
            func(node->value);

            if (auto node_it = node->children.find(*path_it); node_it != node->children.end()) {
                node = std::addressof(node_it->second);
            } else {
                return nullptr;
            }
        }
        return node;
    }

    [[nodiscard]] constexpr node_type *find_or_create(auto path_first, auto path_last) noexcept
    {
        auto *node = &_root;
        for (auto path_it = path_first; path_it != path_last; ++path_it) {
            node = &node->children[*path_it];
        }
        return node;
    }

    /** Call a function on all the child-nodes recursively from the given node.
     *
     * @param node The start node.
     * @param func The function `(value_type &) -> void` to be called for each node.
     */
    constexpr void _walk(node_type *node, auto const& func) noexcept
    {
        func(node->value);
        for (auto& child : node->children) {
            _walk(&child.second, func);
        }
    }

    /** Call a function on all the child-nodes recursively from the given node.
     *
     * @param node The start node.
     * @param func The function `(value_type const &) -> void` to be called for each node.
     */
    constexpr void _walk(node_type const *node, auto const& func) const noexcept
    {
        func(node->value);
        for (auto& child : node->children) {
            _walk(&child.second, func);
        }
    }
};

} // namespace hi::inline v1
