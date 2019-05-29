//---------------------------------------------------------------------------
/// \file   flat_map.hpp
/// \brief  Replacement for boost::flat_map (VS2013 workaround)
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once
// Because VS2013 can't compile boost::flat_map, we use this one as a
// temporary fix.

#include <algorithm>
#include <functional>
#include <vector>
#include <utility>

namespace rhea
{

namespace detail
{

template <class value_type, class C>
class flat_map_compare : public C
{
    typedef std::pair<typename C::_Ty, value_type> Data;
    typedef typename C::_Ty first_argument_type;

public:
    flat_map_compare() {}

    flat_map_compare(const C& src)
        : C(src)
    {
    }

    bool operator()(const first_argument_type& lhs,
                    const first_argument_type& rhs) const
    {
        return C::operator()(lhs, rhs);
    }

    bool operator()(const Data& lhs, const Data& rhs) const
    {
        return operator()(lhs.first, rhs.first);
    }

    bool operator()(const Data& lhs, const first_argument_type& rhs) const
    {
        return operator()(lhs.first, rhs);
    }

    bool operator()(const first_argument_type& lhs, const Data& rhs) const
    {
        return operator()(lhs, rhs.first);
    }
};
}

template <class K, class V, class C = std::less<K>,
          class A = std::allocator<std::pair<K, V>>>
class flat_map : private std::vector<std::pair<K, V>, A>,
                 private detail::flat_map_compare<V, C>
{
    typedef std::vector<std::pair<K, V>, A> base_type;
    typedef detail::flat_map_compare<V, C> compare_type;

public:
    typedef K key_type;
    typedef V mapped_type;
    typedef typename base_type::value_type value_type;

    typedef C key_compare;
    typedef A allocator_type;
    typedef typename A::reference reference;
    typedef typename A::const_reference const_reference;
    typedef typename base_type::iterator iterator;
    typedef typename base_type::const_iterator const_iterator;
    typedef typename base_type::size_type size_type;
    typedef typename base_type::difference_type difference_type;
    typedef typename A::pointer pointer;
    typedef typename A::const_pointer const_pointer;
    typedef typename base_type::reverse_iterator reverse_iterator;
    typedef typename base_type::const_reverse_iterator const_reverse_iterator;

    typedef flat_map<K, V, C, A> self;

    class value_compare
        : public std::binary_function<value_type, value_type, bool>,
          private key_compare
    {
        friend class flat_map;

    protected:
        value_compare(key_compare pred)
            : key_compare(pred)
        {
        }

    public:
        bool operator()(const value_type& lhs, const value_type& rhs) const
        {
            return key_compare::operator()(lhs.first, rhs.first);
        }
    };

    explicit flat_map(const key_compare& comp = key_compare(),
                      const A& alloc = A())
        : base_type(alloc)
        , compare_type(comp)
    {
    }

    template <class InputIterator>
    flat_map(InputIterator first, InputIterator last,
             const key_compare& comp = key_compare(), const A& alloc = A())
        : base_type(first, last, alloc)
        , compare_type(comp)
    {
        compare_type& me = *this;
        std::sort(begin(), end(), me);
    }

    flat_map(self&& m)
        : base_type(std::move(m))
    {
    }

    flat_map(const self& m)
        : base_type(m)
    {
    }

    flat_map& operator=(const flat_map& rhs)
    {
        base_type::operator=(rhs);
        return *this;
    }

    flat_map& operator=(flat_map&& rhs)
    {
        base_type::operator=(std::move(rhs));
        return *this;
    }

    iterator begin() { return base_type::begin(); }
    const_iterator begin() const { return base_type::begin(); }
    iterator end() { return base_type::end(); }
    const_iterator end() const { return base_type::end(); }
    reverse_iterator rbegin() { return base_type::rbegin(); }
    const_reverse_iterator rbegin() const { return base_type::rbegin(); }
    reverse_iterator rend() { return base_type::rend(); }
    const_reverse_iterator rend() const { return base_type::rend(); }

    bool empty() const { return base_type::empty(); }
    size_type size() const { return base_type::size(); }
    size_type max_size() { return base_type::max_size(); }

    mapped_type& operator[](const key_type& key)
    {
        return insert(value_type(key, mapped_type())).first->second;
    }

    std::pair<iterator, bool> insert(const value_type& val)
    {
        bool new_element = false;
        iterator i = lower_bound(val.first);

        if (i == end() || this->operator()(val.first, i->first)) {
            i = base_type::insert(i, val);
            new_element = true;
        }
        return std::make_pair(i, new_element);
    }

    std::pair<iterator, bool> insert(value_type&& val)
    {
        bool new_element = false;
        iterator i = lower_bound(val.first);

        if (i == end() || this->operator()(val.first, i->first)) {
            i = base_type::insert(i, std::move(val));
            new_element = true;
        }
        return std::make_pair(i, new_element);
    }

    iterator insert(iterator pos, const value_type& val)
    {
        if ((pos == begin() || this->operator()(*(pos - 1), val))
            && (pos == end() || this->operator()(val, *pos))) {
            return base_type::insert(pos, val);
        }
        return insert(val).first;
    }

    template <class InputIterator>
    void insert(InputIterator first, InputIterator last)
    {
        for (; first != last; ++first)
            insert(*first);
    }

    void erase(iterator pos) { base_type::erase(pos); }

    size_type erase(const key_type& k)
    {
        iterator i = find(k);
        if (i == end())
            return 0;

        erase(i);
        return 1;
    }

    void erase(iterator first, iterator last)
    {
        base_type::erase(first, last);
    }

    void swap(flat_map& other)
    {
        base_type::swap(other);
        compare_type& me = *this;
        compare_type& rhs = other;
        std::swap(me, rhs);
    }

    void clear() { base_type::clear(); }

    key_compare key_comp() const { return *this; }

    value_compare value_comp() const
    {
        const key_compare& comp = *this;
        return value_compare(comp);
    }

    iterator find(const key_type& k)
    {
        iterator i(lower_bound(k));
        if (i != end() && this->operator()(k, i->first)) {
            i = end();
        }
        return i;
    }

    const_iterator find(const key_type& k) const
    {
        const_iterator i(lower_bound(k));
        if (i != end() && this->operator()(k, i->first)) {
            i = end();
        }
        return i;
    }

    size_type count(const key_type& k) const { return find(k) != end(); }

    iterator lower_bound(const key_type& k)
    {
        compare_type& me = *this;
        return std::lower_bound(begin(), end(), k, me);
    }

    const_iterator lower_bound(const key_type& k) const
    {
        const compare_type& me = *this;
        return std::lower_bound(begin(), end(), k, me);
    }

    iterator upper_bound(const key_type& k)
    {
        compare_type& me = *this;
        return std::upper_bound(begin(), end(), k, me);
    }

    const_iterator upper_bound(const key_type& k) const
    {
        const compare_type& me = *this;
        return std::upper_bound(begin(), end(), k, me);
    }

    std::pair<iterator, iterator> equal_range(const key_type& k)
    {
        compare_type& me = *this;
        return std::equal_range(begin(), end(), k, me);
    }

    std::pair<const_iterator, const_iterator>
    equal_range(const key_type& k) const
    {
        const compare_type& me(*this);
        return std::equal_range(begin(), end(), k, me);
    }

    template <class K1, class V1, class C1, class A1>
    friend bool operator==(const flat_map<K1, V1, C1, A1>& lhs,
                           const flat_map<K1, V1, C1, A1>& rhs);

    bool operator<(const flat_map& rhs) const
    {
        const base_type& me(*this);
        const base_type& yo(rhs);
        return me < yo;
    }

    template <class K1, class V1, class C1, class A1>
    friend bool operator!=(const flat_map<K1, V1, C1, A1>& lhs,
                           const flat_map<K1, V1, C1, A1>& rhs);

    template <class K1, class V1, class C1, class A1>
    friend bool operator>(const flat_map<K1, V1, C1, A1>& lhs,
                          const flat_map<K1, V1, C1, A1>& rhs);

    template <class K1, class V1, class C1, class A1>
    friend bool operator>=(const flat_map<K1, V1, C1, A1>& lhs,
                           const flat_map<K1, V1, C1, A1>& rhs);

    template <class K1, class V1, class C1, class A1>
    friend bool operator<=(const flat_map<K1, V1, C1, A1>& lhs,
                           const flat_map<K1, V1, C1, A1>& rhs);
};

template <class K, class V, class C, class A>
inline bool operator==(const flat_map<K, V, C, A>& lhs,
                       const flat_map<K, V, C, A>& rhs)
{
    const std::vector<std::pair<K, V>, A>& me(lhs);
    return me == rhs;
}

template <class K, class V, class C, class A>
inline bool operator!=(const flat_map<K, V, C, A>& lhs,
                       const flat_map<K, V, C, A>& rhs)
{
    return !(lhs == rhs);
}

template <class K, class V, class C, class A>
inline bool operator>(const flat_map<K, V, C, A>& lhs,
                      const flat_map<K, V, C, A>& rhs)
{
    return rhs < lhs;
}

template <class K, class V, class C, class A>
inline bool operator>=(const flat_map<K, V, C, A>& lhs,
                       const flat_map<K, V, C, A>& rhs)
{
    return !(lhs < rhs);
}

template <class K, class V, class C, class A>
inline bool operator<=(const flat_map<K, V, C, A>& lhs,
                       const flat_map<K, V, C, A>& rhs)
{
    return !(rhs < lhs);
}

template <class K, class V, class C, class A>
void swap(flat_map<K, V, C, A>& lhs, flat_map<K, V, C, A>& rhs)
{
    lhs.swap(rhs);
}

} // namespace rhea
