// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "wfree_idle_count.hpp"
#include "unfair_mutex.hpp"
#include <vector>
#include <tuple>
#include <mutex>

namespace hi::inline v1 {

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
    void read_lock() noexcept
    {
        _idle_count.lock();
    }

    /** Unlock the rcu pointer for reading.
     */
    void read_unlock() noexcept
    {
        _idle_count.unlock();
    }

    /** Lock the rcu pointer for writing.
     */
    void write_lock() noexcept
    {
        _idle_count.lock();
    }

    /** Unlock the rcu pointer for writing.
     */
    void write_unlock() noexcept
    {
        _idle_count.unlock();
    }

    /** get the rcu-pointer.
     */
    value_type const *get() noexcept
    {
        return _ptr.load(std::memory_order::acquire);
    }

    /** Derefence the rcu-pointer.
     *
     * @note This function is unsafe you must follow the rules in
     *       https://github.com/torvalds/linux/blob/master/Documentation/RCU/rcu_dereference.rst
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
     * The version is used to pass to `add_old_copy()`, it should be used while holding a write-lock.
     */
    [[nodiscard]] uint64_t version() const noexcept
    {
        return *_idle_count;
    }

    /** Number of object that are currently allocated.
     */
    [[nodiscard]] size_t capacity() const noexcept
    {
        hilet lock = std::scoped_lock(_old_ptrs_mutex);
        return _old_ptrs.size() + not empty();
    }

    /** Exchange the rcu-pointers.
     *
     * @note This function should be called while holding the write-lock.
     * @param ptr The new pointer value, may be nullptr.
     * @return The old pointer value, may be nullptr.
     */
    [[nodiscard]] value_type *exchange(value_type *ptr) noexcept
    {
        return _ptr.exchange(ptr, std::memory_order::release);
    }

    /** Create a copy of the value.
     */
    [[nodiscard]] value_type *copy() const noexcept
    {
        hilet *allocation = std::allocator_traits<allocator_type>::allocate(_allocator, 1);
        read_lock();
        hilet *new_ptr = std::construct_at(allocation, *get());
        read_unlock();
        return new_ptr;
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
    template<typename... Args>
    void emplace(Args&&...args) noexcept
    {
        auto *const allocation = std::allocator_traits<allocator_type>::allocate(_allocator, 1);
        auto *const new_ptr = std::construct_at(allocation, std::forward<Args>(args)...);

        write_lock();
        auto *const old_ptr = exchange(new_ptr);
        hilet old_version = version();
        write_unlock();

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
        write_lock();
        auto *const old_ptr = _ptr.exchange(nullptr, std::memory_order::release);
        hilet old_version = *_idle_count;
        write_unlock();

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
            std::destroy_at(it->second);
            std::allocator_traits<allocator_type>::deallocate(_allocator, it->second, 1);
            ++it;
        }
        _old_ptrs.erase(_old_ptrs.begin(), it);
    }

private:
    std::atomic<value_type *> _ptr = nullptr;
    mutable wfree_idle_count _idle_count;

    allocator_type _allocator;
    mutable unfair_mutex _old_ptrs_mutex;
    std::vector<std::pair<uint64_t, value_type *>> _old_ptrs;
};

template<typename RCU>
class rcu_read {
public:
    using rcu_type = RCU;
    using value_type = rcu_type::value_type;

    rcu_read(RCU& rcu) noexcept : _rcu(rcu)
    {
        _rcu.read_lock();
        _ptr = _rcu.get();
    }

    ~rcu_read()
    {
        if (_rcu) {
            _rcu.read_unlock();
        }
    }

    constexpr rcu_read() noexcept = default;

    rcu_read(rcu_read const& other) noexcept : _rcu(other._rcu), _ptr(other._ptr)
    {
        _rcu.read_lock();
    }

    rcu_read(rcu_read&& other) noexcept : _rcu(std::exchange(other._rcu, nullptr)), _ptr(std::exchange(other._ptr, nullptr)) {}

    rcu_read& operator=(rcu_read const& other)
    {
        if (_rcu != other._rcu) {
            if (other._rcu) {
                other._rcu->read_lock();
            }
            if (_rcu) {
                _rcu->read_unlock();
            }
        }

        _rcu = other._rcu;
        _ptr = other._ptr;
        _rcu.read_lock();
        return *this;
    }

    rcu_read& operator=(rcu_read&& other) noexcept
    {
        if (_rcu and _rcu != other._rcu) {
            _rcu->read_unlock();
        }

        _rcu = std::exchange(other._rcu, nullptr);
        _ptr = std::exchange(other._ptr, nullptr);
        return *this;
    }

    [[nodiscard]] constexpr value_type const *operator->() const noexcept
    {
        return _ptr;
    }

    [[nodiscard]] constexpr value_type const& operator*() const noexcept
    {
        hi_axiom(_ptr);
        return *_ptr;
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _ptr == nullptr;
    }

    constexpr operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] constexpr bool operator==(nullptr_t) const noexcept
    {
        return _rcu == nullptr;
    }

    void reset() noexcept
    {
        if (_rcu) {
            _rcu.read_unlock();
        }
        _rcu = nullptr;
    }

    rcu_read& operator=(nullptr_t) noexcept
    {
        reset();
        return *this;
    }

private:
    rcu_type *_rcu = nullptr;
    value_type const *_ptr = nullptr;
};

template<typename RCU>
class rcu_write {
public:
    using rcu_type = RCU;
    using value_type = rcu_type::value_type;

    rcu_write(rcu_type const& rcu) noexcept : _rcu(rcu), _ptr(rcu.copy()) {}

    ~rcu_write()
    {
        reset();
    }

    constexpr rcu_write() noexcept = default;
    rcu_write(rcu_write const&) = delete;
    rcu_write& operator=(rcu_write const&) = delete;

    rcu_write(rcu_write&& other) noexcept : _rcu(std::exchange(other._rcu, nullptr)), _ptr(std::exchange(other._ptr, nullptr)) {}

    rcu_write& operator=(rcu_write&& other) noexcept
    {
        reset();
        _rcu = std::exchange(other._rcu, nullptr);
        _ptr = std::exchange(other._ptr, nullptr);
        return *this;
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _ptr == nullptr;
    }

    constexpr operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] constexpr bool operator==(nullptr_t) const noexcept
    {
        return _ptr == nullptr;
    }

    void reset() noexcept
    {
        if (_rcu) {
            _rcu.write_lock();
            hilet *old_ptr = _rcu.exchange(_ptr);
            hilet *old_version = rcu.version();
            _rcu.write_unlock();

            _rcu.add_old_copy(old_version, old_ptr);
            _rcu.cleanup();
            _rcu = nullptr;
            _ptr = nullptr;
        }
    }

    rcu_write& operator=(nullptr_t) noexcept
    {
        reset();
    }

private:
    rcu_type *_rcu = nullptr;
    value_type *_ptr = nullptr;
};

} // namespace hi::inline v1
