


#pragma once

#include "wfree_idle_count.hpp"
#include "unfair_mutex.hpp"
#include <vector>
#include <tuple>

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

    /** A scoped-pointer to a write into a new value.
     *
     * The current value in the rcu-object is replaced with this value when this pointer is destroyed.
     */
    class pointer {
    public:
        constexpr pointer() noexcept = default;

        ~pointer()
        {
            if (_rcu) {
                hilet old_ptr = _parent._ptr.exchange(_ptr, std::memory_order:release);
                hilet old_version = *_parent._idle_count;
                _parent->_idle_count.unlock();
                _parent->manage_versions(old_version, old_ptr);
             }
        }

        constexpr pointer(pointer &&other) noexcept :
            _ptr(std::exchange(other._ptr, nullptr)), _rcu(std::exchange(other._rcu, nullptr)) {}

        constexpr pointer &operator=(pointer &&other) noexcept
        {
            _ptr = std::exchange(_ptr, nullptr);
            _parent = std::exchange(_parent, nullptr);
            return *this;
        }

        constexpr pointer(value_type *ptr, rcu *parent) noexcept : _ptr(ptr), _parent(parent) {}

        [[nodiscard]] constexpr bool empty() const noexcept
        {
            return not to_bool(_ptr);
        }

        constexpr explicit operator bool() const noexcept
        {
            return not empty();
        }

        value_type *operator->() const noexcept
        {
            return _ptr;
        }

        value_type const &operator*() const noexcept
        {
            return *_ptr;
        }

        void reset() noexcept
        {
            if (_parent) {
                hilet old_ptr = _parent._ptr.exchange(_ptr, std::memory_order:release);
                hilet old_version = *_parent._idle_count;
                _parent->_idle_count.unlock();
                _parent->manage_versions(old_version, old_ptr);

                _ptr = nullptr;
                _parent = nullptr;
             }
        }

        constexpr pointer operator=(nullptr_t) noexcept
        {
            reset();
            return *this;
        }

    private:
        value_type *_ptr = nullptr;
        rcu *_parent = nullptr;
    };

    /** A scoped-pointer for reading the current RCU managed object.
     *
     * This will hold an rcu-lock on the pointers. When the object
     * is destroyed the rcu-lock is released. The pointer can be
     * moved and copied while managing the rcu-lock.
     *
     */
    class const_pointer {
    public:
        constexpr const_pointer() noexcept = default;

        constexpr ~const_pointer()
        {
            if (_idle_count) {
                _idle_count->unlock();
            }
        }

        constexpr const_pointer(const_pointer const &other) noexcept : _ptr(other._ptr), _idle_count(other._idle_count)
        {
            if (_idle_count) {
                _idle_count->lock();
            }
        }


        constexpr const_pointer(const_pointer &&other) noexcept :
            _ptr(std::exchange(other._ptr, nullptr)), _idle_count(std::exchange(other._idle_count, nullptr))
        {
        }

        constexpr const_pointer &operator=(const_pointer const &other) noexcept
        {
            if (_idle_count != other._idle_count) {
                if (other._idle_count) {
                    other._idle_count->lock();
                }
                if (_idle_count) {
                    _idle_count->unlock();
                }
            }
            
            _ptr = other._ptr;
            _idle_count = other._idle_count;    
            return *this;
        }

        constexpr const_pointer &operator=(const_pointer &&other) noexcept
        {
            if (_idle_count and _idle_count != other._idle_count) {
                _idle_count->unlock();
            }
            
            _ptr = std::exchange(other._ptr, nullptr);
            _idle_count = std::exchange(other._idle_count, nullptr);
            return *this;
        }

        const_pointer(value_type const *ptr, wfree_idle_count *idle_count) noexcept : _ptr(ptr), _idle_count(idle_count) {}

        [[nodiscard]] constexpr bool empty() const noexcept
        {
            return not to_bool(_ptr);
        }

        [[nodiscard]] constexpr bool operator==(nullptr_t) const noexcept
        {
            return empty();
        }

        constexpr explicit operator bool() const noexcept
        {
            return not empty();
        }

        value_type const *operator->() const noexcept
        {
            return _ptr;
        }

        value_type const &operator*() const noexcept
        {
            return *_ptr;
        }

        constexpr void reset() noexcept
        {
            if (_idle_count) {
                _idle_count->unlock();
            }
            _ptr = nullptr;
            _idle_count = nullptr;
        }

        constexpr pointer operator=(nullptr_t) noexcept
        {
            reset();
            return *this;
        }

        [[nodiscard]] constexpr friend bool operator==(const_pointer const &, const_pointer const &) noexcept;

    private:
        value_type const *_ptr = nullptr;
        wfree_idle_count *_idle_count = nullptr;
    };

    /** Construct a new rcu object.
     *
     * @note The initial rcu will be a nullptr.
     * @param allocator The allocator to use.
     */
    constexpr rcu(allocator_type allocator = allocator_type{}) noexcept _allocator(allocator)

    ~rcu() = default;
    rcu(rcu const &) = delete;
    rcu(rcu &&) = delete;
    rcu &operator=(rcu const &) = delete;
    rcu &operator=(rcu &&) = delete;

    /** Get a read-only pointer the the value.
     *
     * @return A const-pointer to the current value. 
     */
    [[nodiscard]] const_pointer read() const noexcept
    {
        _idle_count.lock();
        return const_pointer(_ptr.load(std::memory_order::acquire), &_idle_count);
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
    tempate<typename... Args>
    void emplace(Args &&... args) noexcept
    {
        hilet *allocation = std::allocator_traits<allocator_type>::allocate(_allocator, 1);
        hilet *new_ptr = std::construct_at(allocation, std::forward<Args>(args)...);

        _idle_count.lock();
        hilet *old_ptr = _ptr.exchange(new_ptr, std::memory_order::release);
        hilet old_version = *_idle_count;
        _idle_count.unlock();

        manage_versions(old_version, old_ptr);
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
        _idle_count.lock();
        hilet *old_ptr = _ptr.exchange(nullptr, std::memory_order::release);
        hilet old_version = *_idle_count;
        _idle_count.unlock();

        manage_versions(old_version, old_ptr);
    }

    rcu &operator=(nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    /** Create a copy of the current value for updating.
     *
     * This function will allocate and copy-construct a new value from the
     * current value.
     *
     * This function will also destroy and deallocate old values when no
     * other threads are reading them.
     *
     * @return A pointer to a copy of the value, yhis value can be modified.
     *         When the pointer is destroyed the modified value will replace
     *         the current value.
     */
    [[nodiscard]] pointer copy() noexcept
    {
        _idle_count.lock();

        hilet *allocation = std::allocator_traits<allocator_type>::allocate(_allocator, 1);
        hilet *new_ptr = std::construct_at(allocation, *_ptr.load(std::memory_order::acquire));

        return proxy(new_ptr, this);
    }

private:
    std::atomic<value_type *> _ptr = nullptr;
    mutable wfree_idle_count _idle_count;

    allocator_type _allocator;
    mutable unfair_mutex _old_ptrs_mutex;
    std::vector<std::pair<uint64_t,value_type *> _old_ptrs;


    void manage_versions(uint64_t old_version, value_type *old_ptr) noexcept
    {
        if (not old_ptr) {
            return;
        }

        hilet new_version = *_idle_count;

        hilet lock = std::scoped_lock(_old_ptrs_mutex);
        _old_ptrs.emplace_back(old_version, old_ptr);

        // Destroy all objects from previous idle-count versions.
        auto it = _old_ptrs.begin();
        while (it !=  _old_ptrs.end() and it->first < new_version) {
            std::destroy_at(it->second);
            std::allocator_traits<allocator_type>::deallocate(_allocator, it->second, 1);
            ++it;
        }
        _old_ptrs.erase(_old_ptrs.begin, it);

    }
};

}


