// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <vector>
#include <tuple>
#include <mutex>
#include <memory>

export module hikogui_concurrency_rcu;
import hikogui_concurrency_unfair_mutex;
import hikogui_concurrency_wfree_idle_count;
import hikogui_utility;


export namespace hi::inline v1 {

/** Read-copy-update.
 *
 * @tparam T The type managed by RCU.
 * @tparam Allocator The allocator used to allocate objects of type T.
 */
template<typename T, typename Allocator = std::allocator<T>>
class rcu {
public:
    using value_type = T;
    using allocator_type = Allocator;

    /** Construct a new rcu object.
     *
     * @note The initial rcu will be a nullptr.
     * @param allocator The allocator to use.
     */
    constexpr rcu(allocator_type allocator = allocator_type{}) noexcept : _allocator(allocator) {}

    ~rcu() = default;
    rcu(rcu const&) = delete;
    rcu(rcu&&) = delete;
    rcu& operator=(rcu const&) = delete;
    rcu& operator=(rcu&&) = delete;

    /** Lock the rcu pointer for reading.
     */
    void lock() const noexcept
    {
        _idle_count.lock();
    }

    /** Unlock the rcu pointer for reading.
     */
    void unlock() const noexcept
    {
        _idle_count.unlock();
    }

    /** get the rcu-pointer.
     *
     * @note A lock on the RCU should be held while dereferencing the returned pointer.
     * @return a const pointer to the current value.
     */
    value_type const *get() const noexcept
    {
        return _ptr.load(std::memory_order::acquire);
    }

    /** get the rcu-pointer.
     *
     * @note A lock on the RCU should be held while dereferencing the returned pointer.
     * @note This function is unsafe you must follow the rules in
     *       https://github.com/torvalds/linux/blob/master/Documentation/RCU/rcu_dereference.rst
     * @return a const pointer to the current value.
     */
    value_type const *unsafe_get() noexcept
    {
#if defined(__alpha__) or defined(__alpha) or defined(_M_ALPHA)
        return _ptr.load(std::memory_order::acquire);
#else
        return _ptr.load(std::memory_order::relaxed);
#endif
    }

    /** The version of the lock.
     *
     * @note This function should be called while holding the lock.
     * @return a version number used for `add_old_copy()`.
     */
    [[nodiscard]] uint64_t version() const noexcept
    {
        return *_idle_count;
    }

    /** Number of objects that are currently allocated.
     *
     * This function is useful in tests to determine if the old copies are properly deallocated.
     */
    [[nodiscard]] size_t capacity() const noexcept
    {
        hilet lock = std::scoped_lock(_old_ptrs_mutex);
        return _old_ptrs.size() + not empty();
    }

    /** Exchange the rcu-pointers.
     *
     * @note This function should be called while holding the lock.
     * @param ptr The new pointer value, may be nullptr.
     * @return The old pointer value, may be nullptr.
     */
    [[nodiscard]] value_type *exchange(value_type *ptr) noexcept
    {
        return _ptr.exchange(ptr, std::memory_order::release);
    }

    /** Create a copy of the value at the given pointer.
     * 
     * @note This function should be called while holding the lock.
     * @note It is undefined behavior to pass a nullptr as the argument
     * @param ptr The pointer to a value to copy.
     * @return A pointer to newly allocated and copy constructed value.
     */
    [[nodiscard]] value_type *copy(value_type const *ptr) const noexcept
    {
        hi_assert_not_null(ptr);
        value_type *new_ptr = std::allocator_traits<allocator_type>::allocate(_allocator, 1);
        std::allocator_traits<allocator_type>::construct(_allocator, new_ptr, *ptr);
        return std::launder(new_ptr);
    }

    /** Create a copy of the value.
     *
     * @note This function takes an internal lock during copying of the current rcu-value.
     * @note It is undefined behavior if the internal value is a nullptr.
     * @return A pointer to newly allocated value and copy constructed from the current rcu-value.
     */
    [[nodiscard]] value_type *copy() const noexcept
    {
        auto *new_ptr = std::allocator_traits<allocator_type>::allocate(_allocator, 1);
        lock();
        value_type const * const ptr = get();
        hi_assert_not_null(ptr);
        std::allocator_traits<allocator_type>::construct(_allocator, new_ptr, *ptr);
        unlock();
        return std::launder(new_ptr);
    }

    /** Abort a copy.
     *
     * @param ptr The pointer returned by `copy()`.
     */
    void abort(value_type *ptr) const noexcept
    {
        std::allocator_traits<allocator_type>::destroy(_allocator, ptr);
        std::allocator_traits<allocator_type>::deallocate(_allocator, ptr, 1);
    }

    /** Commit the copied value.
     *
     * @param ptr The pointer returned by `copy()`.
     */
    void commit(value_type *ptr) noexcept
    {
        lock();
        auto *old_ptr = exchange(ptr);
        auto old_version = version();
        unlock();
        add_old_copy(old_version, old_ptr);
    }

    /** Emplace a new value.
     *
     * This function will allocate and construct a new value, then replace the
     * current value.
     *
     * This function will also destroy and deallocate old values when no
     * other threads are reading them.
     *
     * @param args The arguments passed to the constructor of the value.
     */
    void emplace(auto&&...args) noexcept
    {
        value_type *const new_ptr = std::allocator_traits<allocator_type>::allocate(_allocator, 1);
        std::allocator_traits<allocator_type>::construct(_allocator, new_ptr, hi_forward(args)...);

        lock();
        auto *const old_ptr = exchange(new_ptr);
        hilet old_version = version();
        unlock();

        add_old_copy(old_version, old_ptr);
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return not to_bool(_ptr.load(std::memory_order::relaxed));
    }

    explicit operator bool() const noexcept
    {
        return not empty();
    }

    void reset() noexcept
    {
        lock();
        auto *const old_ptr = _ptr.exchange(nullptr, std::memory_order::release);
        hilet old_version = *_idle_count;
        unlock();

        add_old_copy(old_version, old_ptr);
    }

    rcu& operator=(nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    /** Add an old copy.
     *
     * This function will manage old copies. It will keep a list of
     * copies that are still being used, and destroy and deallocate old copies
     * that are no longer being used.
     *
     * @param old_version The version when the pointer was exchanged.
     * @param old_ptr The pointer that was exchanged, may be a nullptr.
     */
    void add_old_copy(uint64_t old_version, value_type *old_ptr) noexcept
    {
        if (not old_ptr) {
            return;
        }

        hilet new_version = version();

        hilet lock = std::scoped_lock(_old_ptrs_mutex);
        _old_ptrs.emplace_back(old_version, old_ptr);

        // Destroy all objects from previous idle-count versions.
        auto it = _old_ptrs.begin();
        while (it != _old_ptrs.end() and it->first < new_version) {
            std::allocator_traits<allocator_type>::destroy(_allocator, it->second);
            std::allocator_traits<allocator_type>::deallocate(_allocator, it->second, 1);
            ++it;
        }
        _old_ptrs.erase(_old_ptrs.begin(), it);
    }

private:
    std::atomic<value_type *> _ptr = nullptr;
    mutable wfree_idle_count _idle_count;

    mutable allocator_type _allocator;
    mutable unfair_mutex _old_ptrs_mutex;
    std::vector<std::pair<uint64_t, value_type *>> _old_ptrs;
};

} // namespace hi::inline v1

