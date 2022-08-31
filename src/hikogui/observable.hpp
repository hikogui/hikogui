// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "tree.hpp"
#include "notifier.hpp"
#include <memory>
#include <vector>
#include <string>

namespace hi::inline v1 {

template<typename, typename>
class merge_ptr;

/**
 *
 * @tparam T The type of the derived class.
 */
template<typename T, typename D = void *>
class enable_merge_ptr {
public:
    void notify_merge_ptr_owners(D const& data = D{}) const noexcept
    {
        for (auto owner : _emp_owners) {
            hi_axiom(owner != nullptr);
            owner->_notify(data);
        }
    }

private:
    std::vector<merge_ptr<T, D> *> _emp_owners;

    void _emp_add_owner(merge_ptr<T, D> *owner) noexcept
    {
        _emp_owners.push_back(owner);
    }

    void _emp_remove_owner(merge_ptr<T, D> *owner) noexcept
    {
        std::erase(_emp_owners, owner);
    }

    void _emp_reseat(std::shared_ptr<enable_merge_ptr> const& replacement) noexcept
    {
        hi_axiom(replacement.get() != this);

        while (not _emp_owners.empty()) {
            auto owner = _emp_owners.back();
            _emp_owners.pop_back();

            owner->_ptr = replacement;
            if (owner->_ptr) {
                owner->_ptr->_emp_add_owner(owner);
            }
        }
    }

    friend class merge_ptr<T, D>;
};

/**
 *
 * @tparam S the type containing the `merge_ptr`.
 * @tparam T the type that the `merge_ptr` is pointing at.
 */
template<typename T, typename D = void *>
class merge_ptr {
public:
    using element_type = T;

    virtual ~merge_ptr()
    {
        if (_ptr) {
            _ptr->_emp_remove_owner(this);
        }
    }

    merge_ptr(merge_ptr const& other) noexcept : _ptr(other._ptr), _notify()
    {
        if (_ptr) {
            _ptr->_emp_add_owner(this);
        }
    }

    merge_ptr& operator=(merge_ptr const& other) noexcept
    {
        if (_ptr == other._ptr) {
            return *this;

        } else if (_ptr) {
            _ptr->_emp_reseat(other._ptr);
            return *this;

        } else {
            _ptr = other._ptr;
            _ptr->_emp_add_owner(this);
            return *this;
        }
    }

    merge_ptr(merge_ptr&& other) noexcept : _ptr(std::move(other._ptr)), _notify()
    {
        if (_ptr) {
            _ptr->_emp_remove_owner(&other);
            _ptr->_emp_add_owner(this);
        }
    }

    merge_ptr& operator=(merge_ptr&& other) noexcept
    {
        hi_return_on_self_assignment(other);

        if (other._ptr) {
            other._ptr->_emp_remove_owner(&other);
        }

        if (_ptr == other._ptr) {
            other._ptr = nullptr;
            return *this;

        } else if (_ptr) {
            _ptr->_emp_reseat(std::exchange(other._ptr, nullptr));
            return *this;

        } else {
            _ptr = std::exchange(other._ptr, nullptr);
            _ptr->_emp_add_owner(this);
            return *this;
        }
    }

    constexpr merge_ptr() noexcept : _ptr(), _notify() {}
    constexpr merge_ptr(std::nullptr_t) noexcept : _ptr(), _notify() {}

    void reset() noexcept
    {
        if (_ptr) {
            _ptr->_emp_remove_owner(this);
            _ptr = nullptr;
        }
    }

    merge_ptr(std::shared_ptr<enable_merge_ptr<T, D>> const& ptr) noexcept : _ptr(ptr)
    {
        if (_ptr) {
            _ptr->_emp_add_owner(this);
        }
    }

    merge_ptr(std::shared_ptr<enable_merge_ptr<T, D>>&& ptr) noexcept : _ptr(std::move(ptr))
    {
        if (_ptr) {
            _ptr->_emp_add_owner(this);
        }
    }

    merge_ptr& operator=(std::shared_ptr<enable_merge_ptr<T, D>> const& ptr) noexcept
    {
        return *this = merge_ptr{ptr};
    }

    merge_ptr& operator=(std::shared_ptr<enable_merge_ptr<T, D>> && ptr) noexcept
    {
        return *this = merge_ptr{std::move(ptr)};
    }

    [[nodiscard]] T *get() const noexcept
    {
        return static_cast<T *>(_ptr.get());
    }

    [[nodiscard]] T *operator->() const noexcept
    {
        return get();
    }

    [[nodiscard]] T& operator*() const noexcept
    {
        return *get();
    }

    explicit operator bool() const noexcept
    {
        return _ptr;
    }

    void subscribe(forward_of<void(D)> auto&& func) noexcept
    {
        _notify = hi_forward(func);
    }

private:
    std::shared_ptr<enable_merge_ptr<T, D>> _ptr;
    std::function<void(D)> _notify;

    friend class enable_merge_ptr<T, D>;
};

template<typename Context, typename... Expected>
struct is_forward_of<Context, merge_ptr<Expected...>> :
    std::conditional_t<std::is_convertible_v<Context, merge_ptr<Expected...>>, std::true_type, std::false_type> {
};

struct observable_msg {
    /** The type of the path used for notifying observers.
     */
    using path_type = std::vector<std::string>;

    void const * const ptr;
    path_type const& path;
};

/** An abstract observable object.
 *
 * This type is referenced by `observer`s
 */
class observable : public enable_merge_ptr<observable, observable_msg> {
public:
    constexpr virtual ~observable() = default;
    observable(observable const&) = delete;
    observable(observable&&) = delete;
    observable& operator=(observable const&) = delete;
    observable& operator=(observable&&) = delete;
    constexpr observable() noexcept = default;

    /** Get a pointer to the current value.
     *
     * @note `read()` does not `read_lock()` the observable and should be done before `read()`.
     * @return A const pointer to the value. The `observer` should cast this to a pointer to the value-type.
     */
    [[nodiscard]] virtual void const *read() const noexcept = 0;

    /** Allocate and make a copy of the value.
     *
     * @note `copy()` does not `write_lock()` the observable and should be done before `read()`.
     * @param A pointer to the value that was `read()`.
     * @return A pointer to a newly allocated copy of the value.
     */
    [[nodiscard]] virtual void *copy(void const *ptr) const noexcept = 0;

    /** Commit the modified copy.
     *
     * @note `commit()` does not `write_unlock()`.
     * @param ptr A pointer to the modified new value returned by `copy()`.
     */
    virtual void commit(void *ptr) noexcept = 0;

    /** Abort the modified copy.
     *
     * @note `abort()` does not `write_unlock()`.
     * @param ptr A pointer to the modified new value returned by `copy()`.
     */
    virtual void abort(void *ptr) const noexcept = 0;

    /** Lock for reading.
     */
    virtual void read_lock() const noexcept = 0;

    /** Unlock for reading.
     */
    virtual void read_unlock() const noexcept = 0;

    /** Lock for writing.
     */
    virtual void write_lock() const noexcept = 0;

    /** Unlock for writing.
     */
    virtual void write_unlock() const noexcept = 0;
};

} // namespace hi::inline v1
