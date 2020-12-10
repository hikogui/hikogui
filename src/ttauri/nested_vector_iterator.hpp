// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <type_traits>
#include <vector>

namespace tt
{


/** An iterator for a vector inside another vector.
 */
template<
    typename ParentCIt,
    typename ParentIt,
    typename ChildIt=std::conditional_t<
        std::is_const_v<std::remove_reference_t<typename std::iterator_traits<ParentIt>::reference>>,
        typename ParentIt::value_type::const_iterator,
        typename ParentIt::value_type::iterator
    >
>
class nested_vector_iterator {
    using value_type = typename std::iterator_traits<ChildIt>::value_type;
    using difference_type = typename std::iterator_traits<ChildIt>::difference_type;
    using reference = typename std::iterator_traits<ChildIt>::reference;

    ParentCIt parent_it_end;
    ParentIt parent_it;
    ChildIt child_it;

public:
    nested_vector_iterator() noexcept = delete;
    nested_vector_iterator(nested_vector_iterator const &other) noexcept = default;
    nested_vector_iterator(nested_vector_iterator &&other) noexcept = default;
    nested_vector_iterator &operator=(nested_vector_iterator const &other) noexcept = default;
    nested_vector_iterator &operator=(nested_vector_iterator &&other) noexcept = default;
    ~nested_vector_iterator() = default;

    nested_vector_iterator(ParentCIt parent_it_end, ParentIt parent_it, ChildIt child_it) noexcept :
        parent_it_end(parent_it_end), parent_it(parent_it), child_it(child_it) {}

    nested_vector_iterator(ParentCIt parent_it_end, ParentIt parent_it) noexcept :
        parent_it_end(parent_it_end), parent_it(parent_it)
    {
        tt_axiom(parent_it_end == parent_it);
    }

    /** Get the current parent iterator.
     */
    ParentIt parent() const noexcept { return parent_it; }

    /** Don't need to check the child_it at end.
     */
    [[nodiscard]] bool at_end() const noexcept { return parent_it == parent_it_end; }

    template<typename X=std::remove_reference_t<reference>, std::enable_if_t<!std::is_const_v<X>, int> = 0>
    [[nodiscard]] value_type &operator*() noexcept { return *child_it; }

    [[nodiscard]] value_type const &operator*() const noexcept { return *child_it; }

    template<typename X=std::remove_reference_t<reference>, std::enable_if_t<!std::is_const_v<X>, int> = 0>
    [[nodiscard]] value_type *operator->() noexcept { return &(*child_it); }

    [[nodiscard]] value_type const *operator->() const noexcept { return &(*child_it); }

    template<typename X=std::remove_reference_t<reference>, std::enable_if_t<!std::is_const_v<X>, int> = 0>
    [[nodiscard]] value_type &operator[](size_t i) noexcept { return *(*this + i); }

    [[nodiscard]] value_type const &operator[](size_t i) const noexcept { return *(*this + i); }

    nested_vector_iterator &operator++() noexcept {
        ++child_it;
        if (child_it == parent_it->cend()) {
            ++parent_it;
            if (!at_end()) {
                child_it = parent_it->begin();
            }
        }
        return *this;
    }

    nested_vector_iterator &operator--() noexcept {
        if (child_it == parent_it->begin()) {
            --parent_it;
            child_it = parent_it->end();
        }
        --child_it;
        return *this;
    }

    nested_vector_iterator operator++(int) noexcept { auto tmp = *this; ++(*this); return tmp; }
    nested_vector_iterator operator--(int) noexcept { auto tmp = *this; --(*this); return tmp; }

    nested_vector_iterator &operator+=(difference_type rhs) noexcept {
        if (rhs < 0) {
            return (*this) -= -rhs;
        }

        do {
            ttlet line_left = std::distance(child_it, parent_it->end());

            if (line_left < rhs) {
                ++parent_it;
                if (!at_end()) {
                    child_it = parent_it->begin();
                }
                rhs -= line_left;

            } else {
                child_it += rhs;
                rhs -= rhs;
            }

        } while (rhs);

        return *this;
    }

    nested_vector_iterator &operator-=(difference_type rhs) noexcept {
        if (rhs < 0) {
            return (*this) += -rhs;
        }

        do {
            ttlet line_left = std::distance(parent_it->begin(), child_it) + 1;

            if (line_left < rhs) {
                --parent_it;
                child_it = parent_it->end() - 1;
                rhs -= line_left;

            } else {
                child_it -= rhs;
                rhs -= rhs;
            }

        } while (rhs);

        return *this;
    }

    [[nodiscard]] friend bool operator==(nested_vector_iterator const &lhs, nested_vector_iterator const &rhs) noexcept {
        if (lhs.at_end() && rhs.at_end()) {
            return true;
        } else if (lhs.at_end() || rhs.at_end()) {
            return false;
        } else {
            return (lhs.parent_it == rhs.parent_it) && (lhs.child_it == rhs.child_it);
        }
    }

    [[nodiscard]] friend bool operator<(nested_vector_iterator const &lhs, nested_vector_iterator const &rhs) noexcept {
        if (lhs.at_end() && rhs.at_end()) {
            return false;
        } else if (rhs.at_end()) {
            return true;
        } else if (lhs.at_end()) {
            return false;
        } else if (lhs.parent_it == rhs.parent_it) {
            return lhs.child_it < rhs.child_it;
        } else {
            return lhs.parent_it < rhs.parent_it;
        }
    }

    [[nodiscard]] friend bool operator!=(nested_vector_iterator const &lhs, nested_vector_iterator const &rhs) noexcept { return !(lhs == rhs); }
    [[nodiscard]] friend bool operator>(nested_vector_iterator const &lhs, nested_vector_iterator const &rhs) noexcept { return rhs < lhs; }
    [[nodiscard]] friend bool operator<=(nested_vector_iterator const &lhs, nested_vector_iterator const &rhs) noexcept { return !(lhs > rhs); }
    [[nodiscard]] friend bool operator>=(nested_vector_iterator const &lhs, nested_vector_iterator const &rhs) noexcept { return !(lhs < rhs); }

    [[nodiscard]] friend nested_vector_iterator operator+(nested_vector_iterator lhs, difference_type rhs) noexcept { return lhs += rhs; }
    [[nodiscard]] friend nested_vector_iterator operator-(nested_vector_iterator lhs, difference_type rhs) noexcept { return lhs -= rhs; }
    [[nodiscard]] friend nested_vector_iterator operator+(difference_type lhs, nested_vector_iterator rhs) noexcept { return rhs += lhs; }

    [[nodiscard]] friend difference_type operator-(nested_vector_iterator const &lhs, nested_vector_iterator const &rhs) noexcept {
        if (rhs < lhs) {
            return -(rhs - lhs);
        } else {
            auto lhs_ = lhs;
            ssize_t count = 0;
            while (lhs_.parent_it < rhs.parent_it) {
                count += std::distance(lhs_.child_it, lhs_.parent_it->end());
                ++(lhs_.parent_it);
                lhs_.child_it = lhs_.parent_it->begin();
            }
            return count + std::distance(lhs_.child_it, rhs.child_it);
        }
    }
};

template<typename NestedVector>
[[nodiscard]] auto nested_vector_iterator_begin(NestedVector &rhs) noexcept {
    return size(rhs) == 0 ?
        nested_vector_iterator(rhs.cend(), rhs.begin()) :
        nested_vector_iterator(rhs.cend(), rhs.begin(), rhs.front().begin());
}

template<typename NestedVector>
[[nodiscard]] auto nested_vector_iterator_end(NestedVector &rhs) noexcept {
    return size(rhs) == 0 ?
        nested_vector_iterator(rhs.cend(), rhs.end()) :
        nested_vector_iterator(rhs.cend(), rhs.end(), rhs.back().end());
}

template<typename NestedVector>
[[nodiscard]] auto nested_vector_iterator_cbegin(NestedVector const &rhs) noexcept {
    return size(rhs) == 0 ?
        nested_vector_iterator(rhs.cend(), rhs.cbegin()) :
        nested_vector_iterator(rhs.cend(), rhs.cbegin(), rhs.front().cbegin());
}

template<typename NestedVector>
[[nodiscard]] auto nested_vector_iterator_cend(NestedVector const &rhs) noexcept {
    return size(rhs) == 0 ?
        nested_vector_iterator(rhs.cend(), rhs.cend()) :
        nested_vector_iterator(rhs.cend(), rhs.cend(), rhs.back().cend());
}

}
