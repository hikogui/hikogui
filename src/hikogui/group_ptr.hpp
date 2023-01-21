// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility/module.hpp"
#include "concurrency/module.hpp"
#include <memory>
#include <vector>
#include <functional>
#include <tuple>

namespace hi::inline v1 {

template<typename>
class group_ptr;

/** Enable a class to be used in a `group_ptr`.
 *
 * This class has the following public function:
 *  - `notify_group_ptr(...)`
 *
 * `notify_group_ptr(...)` will call the callback with the prototype @a Proto
 * that has been registered with the each `group_ptr` that point to `this`.
 *
 * @note It is undefined behavior to specialize this template.
 * @tparam T The type of the derived class.
 * @tparam Proto the optional prototype of the callback function that is
 *         registered with the `group_ptr` in the form: `void(...)`.
 *         If not specified the default `void()` is used.
 */
template<typename T, typename... Proto>
class enable_group_ptr;

template<typename T, typename... Args>
class enable_group_ptr<T, void(Args...)> {
public:
#ifndef NDEBUG
    ~enable_group_ptr()
    {
        hi_assert(_enable_group_ptr_owners.empty());
    }
#endif

    /** Call the callback which are registered with the owning `group_ptr`s.
     *
     * @param args The arguments to pass to the callback function.
     */
    void notify_group_ptr(Args const&...args) const noexcept
    {
        hilet owners_copy = [&] {
            hilet lock = std::scoped_lock(_enable_group_ptr_mutex);
            return _enable_group_ptr_owners;
        }();

        for (auto owner : owners_copy) {
            hi_assert_not_null(owner);
            if (owner->_notify) {
                owner->_notify(args...);
            }
        }
    }

private:
    using _enable_group_ptr_notify_proto = void(Args...);

    std::vector<group_ptr<T> *> _enable_group_ptr_owners;
    mutable unfair_mutex _enable_group_ptr_mutex;

    [[nodiscard]] bool _enable_group_ptr_holds_invariant() const noexcept
    {
        hi_axiom(_enable_group_ptr_mutex.is_locked());

        for (auto owner : _enable_group_ptr_owners) {
            if (owner == nullptr or owner->_ptr == nullptr or owner->_ptr.get() != this or
                owner->_ptr.use_count() < _enable_group_ptr_owners.size()) {
                return false;
            }
        }
        return true;
    }

    void _enable_group_ptr_add_owner(group_ptr<T> *owner) noexcept
    {
        hilet lock = std::scoped_lock(_enable_group_ptr_mutex);

        _enable_group_ptr_owners.push_back(owner);
        hi_axiom(_enable_group_ptr_holds_invariant());
    }

    void _enable_group_ptr_remove_owner(group_ptr<T> *owner) noexcept
    {
        hilet lock = std::scoped_lock(_enable_group_ptr_mutex);

        hilet num_removed = std::erase(_enable_group_ptr_owners, owner);
        hi_assert(num_removed == 1);
        hi_axiom(_enable_group_ptr_holds_invariant());
    }

    /** Reseat all the owners with the replacement.
     *
     * @note It is undefined behavior to pass a nullptr to @a owner, or to pass nullptr to @a replacement or @replacement points
     *       to `this`.
     * @param replacement The replacement object.
     */
    void _enable_group_ptr_reseat(std::shared_ptr<enable_group_ptr> const& replacement) noexcept
    {
        hilet lock = std::scoped_lock(_enable_group_ptr_mutex);

        hi_assert_not_null(replacement);
        hi_assert_not_null(replacement.get());

        while (not _enable_group_ptr_owners.empty()) {
            auto *owner = _enable_group_ptr_owners.back();
            _enable_group_ptr_owners.pop_back();
            owner->_ptr = replacement;
            owner->_ptr->_enable_group_ptr_add_owner(owner);
        }
        hi_axiom(_enable_group_ptr_holds_invariant());
    }

    friend class group_ptr<T>;
};

template<typename T>
class enable_group_ptr<T> : public enable_group_ptr<T, void()> {
};

/** A smart pointer which manages ownership as a group.
 *
 * When `group_ptr`s are assigned to one another they will act as
 * a group from this point forward. When a `std::shared_ptr` is as assigned
 * to a `group_ptr` it will now be assigned to each `group_ptr` in this group.
 *
 * You can unlink a `group_ptr` from others in the group only by `reset()`,
 * assigning an empty `std::shared_ptr`, assigning a nullptr or move-assignment/construction.
 *
 * @tparam S the type containing the `group_ptr`.
 * @tparam T the type that the `group_ptr` is pointing at.
 */
template<typename T>
class group_ptr {
public:
    using notify_proto = T::_enable_group_ptr_notify_proto;
    using element_type = T;

    /** Destroy this `group_ptr`.
     */
    virtual ~group_ptr()
    {
        if (_ptr) {
            _ptr->_enable_group_ptr_remove_owner(this);
        }
    }

    /** Copy construct from another `group_ptr`.
     *
     * This function will:
     *  - copy the `std::shared_ptr` from @a other.
     *  - join in the group with @a other.
     *  - make the callback subscription empty.
     *
     * @param other The other group_ptr to copy.
     */
    group_ptr(group_ptr const& other) noexcept : _ptr(other._ptr)
    {
        if (_ptr) {
            _ptr->_enable_group_ptr_add_owner(this);
        }
    }

    /** Copy assign from another `group_ptr`.
     *
     * If other is non-empty this function will:
     *  - assign the pointer from @a other to all members of the group.
     *  - all members of the group will join the group of @a other.
     *  - leave the callback subscription as-is.
     *
     * If other is empty this function will:
     *  - leave the group.
     *  - make the pointer empty.
     *  - leave the callback subscription as-is.
     *
     * @param other The other group_ptr to copy.
     * @return a reference to this.
     */
    group_ptr& operator=(group_ptr const& other) noexcept
    {
        if (_ptr == other._ptr) {
            return *this;

        } else if (_ptr and other._ptr) {
            // Make copy because reseat() will overwrite _ptr.
            auto tmp = _ptr;
            tmp->_enable_group_ptr_reseat(other._ptr);
            return *this;

        } else if (_ptr) {
            _ptr->_enable_group_ptr_remove_owner(this);
            _ptr = nullptr;
            return *this;

        } else {
            _ptr = other._ptr;
            _ptr->_enable_group_ptr_add_owner(this);
            return *this;
        }
    }

    /** Move construct from another `group_ptr`.
     *
     * This function will:
     *  - move the `std::shared_ptr` from @a other.
     *  - join in the group with @a other.
     *  - @a other leaves the group.
     *  - make the callback subscription empty.
     *
     * @param other The other group_ptr to copy.
     */
    group_ptr(group_ptr&& other) noexcept : _ptr(std::move(other._ptr))
    {
        if (_ptr) {
            _ptr->_enable_group_ptr_remove_owner(&other);
            _ptr->_enable_group_ptr_add_owner(this);
        }
    }

    /** Move assign from another `group_ptr`.
     *
     * If other is non-empty this function will:
     *  - assign the pointer from @a other to all members of the group.
     *  - all members of the group will join the group of @a other.
     *  - @a other will leave the group.
     *  - leave the callback subscription as-is.
     *
     * If other is empty this function will:
     *  - leave the group.
     *  - make the pointer empty.
     *  - leave the callback subscription as-is.
     *
     * @param other The other group_ptr to copy.
     * @return a reference to this.
     */
    group_ptr& operator=(group_ptr&& other) noexcept
    {
        hi_return_on_self_assignment(other);

        if (_ptr == other._ptr) {
            other._ptr->_enable_group_ptr_remove_owner(&other);
            other._ptr = nullptr;
            return *this;

        } else if (_ptr and other._ptr) {
            other._ptr->_enable_group_ptr_remove_owner(&other);
            // Make copy because reseat() will overwrite _ptr.
            auto tmp = _ptr;
            tmp->_enable_group_ptr_reseat(std::exchange(other._ptr, nullptr));
            return *this;

        } else if (_ptr) {
            _ptr->_enable_group_ptr_remove_owner(this);
            _ptr = nullptr;
            return *this;

        } else {
            other._ptr->_enable_group_ptr_remove_owner(&other);
            _ptr = std::exchange(other._ptr, nullptr);
            _ptr->_enable_group_ptr_add_owner(this);
            return *this;
        }
    }

    /** Construct an empty group_ptr.
     */
    constexpr group_ptr() noexcept = default;

    /** Construct an empty group_ptr.
     */
    constexpr group_ptr(std::nullptr_t) noexcept {}

    /** Reset the group_ptr and make it empty.
     *
     * This function will unlink the `group_ptr` from other `group_ptr`s.
     *
     * This function will not reset the subscription of a callback.
     */
    void reset() noexcept
    {
        if (_ptr) {
            _ptr->_enable_group_ptr_remove_owner(this);
            _ptr = nullptr;
        }
    }

    /** Construct a group_ptr from a shared_ptr.
     *
     * @param ptr An object that is convertible to a `std::shared_ptr` pointing to
     *            and object that is compatible with this `group_ptr`.
     *
     * This function will not reset the subscription of a callback.
     */
    template<forward_of<std::shared_ptr<enable_group_ptr<T, notify_proto>>> Ptr>
    group_ptr(Ptr&& ptr) noexcept : _ptr(std::forward<Ptr>(ptr))
    {
        if (_ptr) {
            _ptr->_enable_group_ptr_add_owner(this);
        }
    }

    /** Construct a group_ptr from a shared_ptr.
     *
     * @param ptr An object that is convertible to a `std::shared_ptr` pointing to
     *            and object that is compatible with this `group_ptr`. If the shared_ptr is
     *            empty it will perform the same function as `reset()`.
     *
     * This function will not reset the subscription of a callback.
     */
    template<forward_of<std::shared_ptr<enable_group_ptr<T, notify_proto>>> Ptr>
    group_ptr& operator=(Ptr&& ptr) noexcept
    {
        return *this = group_ptr{std::forward<Ptr>(ptr)};
    }

    /** Get the pointer to the object that this `group_ptr` owns.
     *
     * @return The pointer to object that this `group_ptr` owns. Or nullptr
     *         if the `group_ptr` is empty.
     */
    [[nodiscard]] T *get() const noexcept
    {
        return static_cast<T *>(_ptr.get());
    }

    /** Dereference to member of the object that is owned by this `group_ptr`.
     *
     * @note It is undefined behavior to dereference an empty `group_ptr`.
     * @return A non-const pointer to the object that is owned by this `group_ptr`.
     */
    [[nodiscard]] T *operator->() const noexcept
    {
        return get();
    }

    /** Dereference the object that is owned by this `group_ptr`.
     *
     * @note It is undefined behavior to dereference an empty `group_ptr`.
     * @return A non-const reference to the object that is owned by this `group_ptr`.
     */
    [[nodiscard]] T& operator*() const noexcept
    {
        return *get();
    }

    /** Check if this `group_ptr` is not empty.
     *
     * @retval true When this `group_ptr` is not empty.
     * @retval false When this `group_ptr` is not empty.
     */
    explicit operator bool() const noexcept
    {
        return to_bool(_ptr);
    }

    /** Subscribe a callback function.
     *
     * The subscribed callback function can be called by the
     * `notify_group_ptr()` function of the object that is owned by the `group_ptr`.
     *
     * Only a single function can be subscribed. This function will overwrite a
     * previous subscribed function.
     *
     * @param func A function object corresponding to the `notify_proto` prototype.
     */
    void subscribe(forward_of<notify_proto> auto&& func) noexcept
    {
        _notify = hi_forward(func);
    }

    /** Unsubscribe the callback function.
     */
    void unsubscribe() noexcept
    {
        _notify = {};
    }

private:
    std::shared_ptr<enable_group_ptr<T, notify_proto>> _ptr = {};
    std::function<notify_proto> _notify = {};

    friend class enable_group_ptr<T, notify_proto>;
};

template<typename Context, typename Expected>
struct is_forward_of<Context, group_ptr<Expected>> :
    std::conditional_t<std::is_convertible_v<Context, group_ptr<Expected>>, std::true_type, std::false_type> {
};

} // namespace hi::inline v1
