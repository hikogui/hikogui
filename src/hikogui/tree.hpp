

#pragma once

#include <memory>


namespace hi::inline v1 {

template<typename Key, typename T, typename Compare = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>>
class tree {
public:
    using key_type = Key;
    using mapped_type = T;

    using map_type = std::map<key_type, element_type>;
    using iterator = map_type::iterator;
    using const_iterator = map_type::const_iterator;

    // XXX c++23 multiple argument index operator
    template<typename KeyIt, typename KeyEndIt, typename... Args>
    value_type &operator()(KeyIt key_first, KeyEndIt key_last) noexcept
    {
        return find_or_create(key_first, key_last)->value;
    }

    // XXX c++23 multiple argument index operator
    template<typename KeyIt, typename KeyEndIt, typename... Args>
    value_type const &operator()(KeyIt key_first, KeyEndIt key_last) const noexcept
    {
        return const_cast<tree *>(this)->operator()(key_first, key_last);
    }

    template<typename KeyRange>
    value_type &operator[](KeyRange const &key) noexcept
    {
        using std::cbegin;
        using std::cend;
        return this->operator()(cbegin(key), cend(key));
    }

    template<typename KeyRange>
    value_type const &operator[](KeyRange const &key) const noexcept
    {
        return const_cast<tree *>(this)->operator[](key);
    }

    template<typename KeyIt, typename KeyEndIt, typename Func>
    void walk(KeyIt key_first, KeyEndIt key_last, Func const &func) noexcept
    {
        if (auto element = find(key_first, key_last)) {
            walk(element, func);
        }
    }

    template<typename KeyRange, typename Func>
    void walk(KeyRange const &key, Func const &func) noexcept
    {
        using std::cbegin;
        using std::cend;
        if (auto element = find(cbegin(key), cend(key))) {
            walk(element, func);
        }
    }

    template<typename KeyIt, typename KeyEndIt, typename Func>
    void walk_from_root(KeyIt key_first, KeyEndIt key_last, Func const &func) noexcept
    {
        auto *element = &_root;
        for (auto key = key_first; key != key_last; ++key) {
            element = &element->children[*key];
            func(element->value);
        }

        walk(element, func);
    }

    template<typename KeyRange, typename Func>
    void walk_from_root(KeyRange const &key, Func const &func) noexcept
    {
        using std::cbegin;
        using std::cend;
        if (auto element = find(cbegin(key), cend(key))) {
            walk_from_root(element, func);
        }
    }

    template<typename Func>
    void walk(Func const &func) noexcept
    {
        walk(&_root, func);
    }

private:
    struct element_type {
        value_type value;
        map_type children;
    };

    element_type _root;

    template<typename KeyIt, typename KeyEndIt>
    element_type *find(KeyIt key_first, KeyEndIt key_last) noexcept
    {
        auto *element = &_root;
        for (auto key = key_first; key != key_last; ++key) {
            if (auto it = element->children.find(*key); it != element->children.end()) {
                element = &it->second;
            } else {
                return nullptr;
            }
        }
        return element;
    }

    template<typename KeyIt, typename KeyEndIt>
    element_type *find_or_create(KeyIt key_first, KeyEndIt key_last) noexcept
    {
        auto *element = &_root;
        for (auto key = key_first; key != key_last; ++key) {
            element = &element->childrent[*key];
        }
        return element;
    }

    template<typename Func>
    void walk(element_type *element, Func const &func) noexcept
    {
        for (auto &child: element->children) {
            func(child.value);
            walk(&child);
        }
    }
};

}

