// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <array>
#include <memory>
#include <type_traits>
#include <atomic>
#include <concepts>
#include <thread>
#include <optional>
#include <chrono>

hi_export_module(hikogui.container.polymorphic_optional);

hi_warning_push();
// C26432: If you define or delete any default operation in the type '...', define or delete them all (c.21).
// False positive. The copy/move assignment/constructors are templated
hi_warning_ignore_msvc(26432);
// C26495: Variable '...' is uninitialized. Always initialize a member variable (type.6).
// For performance reasons polymorphic_optional::_buffer must remain uninitialized.
hi_warning_ignore_msvc(26495);
// C26403: Reset or explicitly delete an owner<T> pointer 'new_ptr'
// We can't use std::allocator because we can't hold a size and be compatible with unique_ptr at the same time.
hi_warning_ignore_msvc(26403);

hi_export namespace hi::inline v1 {

/** Polymorphic optional.
 * This optional container can hold an polymorphic value.
 *
 * If the assigned sub-class is larger than the internal buffer
 * the object will be allocated on the heap.
 *
 * @tparam BaseType The base type of the polymorphic value.
 * @tparam Size Total size of the polymorphic_optional internal storage.
 */
template<typename BaseType, std::size_t Size, std::size_t Alignment = alignof(BaseType)>
class alignas(Alignment) polymorphic_optional {
public:
    using base_type = BaseType;
    using pointer = base_type *;
    using const_pointer = base_type const *;

    static_assert(std::atomic<pointer>::is_always_lock_free);

    /** The maximum size of a value that can be placed inside the buffer of this.
     */
    constexpr static std::size_t capacity = Size - sizeof(std::atomic<pointer>);

    /** The alignment of this.
     */
    constexpr static std::size_t alignment = Alignment;

    ~polymorphic_optional()
    {
        reset();
    }

    constexpr polymorphic_optional() noexcept = default;
    constexpr polymorphic_optional(std::nullopt_t) noexcept : polymorphic_optional() {}

    template<std::derived_from<base_type> Other>
    constexpr polymorphic_optional(std::unique_ptr<Other, std::default_delete<Other>>&& other) noexcept :
        _pointer(other.release())
    {
    }

    template<std::derived_from<base_type> Other>
    polymorphic_optional& operator=(std::unique_ptr<Other, std::default_delete<Other>>&& other) noexcept
    {
        reset();
        _pointer.store(other.release(), std::memory_order::release);
    }

    template<std::derived_from<base_type> Other>
    polymorphic_optional(Other&& other) noexcept : _pointer(new Other(std::forward<Other>(*other)))
    {
    }

    template<std::derived_from<base_type> Other>
    polymorphic_optional(Other&& other) noexcept requires(sizeof(Other) <= capacity and alignof(Other) <= alignment) :
        _pointer(new (_buffer.data()) Other(std::forward<Other>(*other)))
    {
    }

    template<std::derived_from<base_type> Other>
    polymorphic_optional& operator=(Other&& other) noexcept
    {
        reset();
        auto *new_ptr = new Other(std::forward<Other>(other));
        _pointer.store(new_ptr, std::memory_order::release);
        return *this;
    }

    template<std::derived_from<base_type> Other>
    polymorphic_optional& operator=(Other&& other) noexcept requires(sizeof(Other) <= capacity and alignof(Other) <= alignment)
    {
        reset();
        auto *new_ptr = new (_buffer.data()) Other(std::forward<Other>(other));
        _pointer.store(new_ptr, std::memory_order::release);
        return *this;
    }

    [[nodiscard]] bool empty(std::memory_order memory_order = std::memory_order::seq_cst) const noexcept
    {
        return _pointer.load(memory_order) == nullptr;
    }

    operator bool() const noexcept
    {
        return not empty();
    }

    template<typename Value>
    Value& value(std::memory_order memory_order = std::memory_order::seq_cst)
    {
        auto *ptr = _pointer.load(memory_order);
        if (ptr == nullptr) {
            throw std::bad_optional_access();
        }
        return down_cast<Value&>(*ptr);
    }

    template<typename Value>
    Value const& value(std::memory_order memory_order = std::memory_order::seq_cst) const
    {
        auto *ptr = _pointer.load(memory_order);
        if (ptr == nullptr) {
            throw std::bad_optional_access();
        }
        return down_cast<Value const&>(*ptr);
    }

    base_type *operator->() noexcept
    {
        return _pointer.load();
    }

    base_type const *operator->() const noexcept
    {
        return _pointer.load();
    }

    base_type& operator*() noexcept
    {
        return *_pointer.load();
    }

    base_type const& operator*() const noexcept
    {
        return *_pointer.load();
    }

    /** Destroys the contained value, otherwise has no effect.
     */
    hi_force_inline void reset() noexcept
    {
        if (auto *ptr = _pointer.exchange(nullptr, std::memory_order::acquire)) {
            if (equal_ptr(ptr, this)) {
                std::destroy_at(ptr);
            } else {
                delete ptr;
            }
        }
    };

    template<typename Value, typename... Args>
    hi_force_inline Value& emplace(Args&&...args) noexcept
    {
        reset();

        if constexpr (sizeof(Value) <= capacity and alignof(Value) <= alignment) {
            // Overwrite the buffer with the new slot.
            auto new_ptr = new (_buffer.data()) Value(std::forward<Args>(args)...);
            hi_axiom(equal_ptr(new_ptr, this));

            _pointer.store(new_ptr, std::memory_order::release);
            return *new_ptr;

        } else {
            // We need a heap allocated pointer with a fully constructed object
            // Lets do this ahead of time to let another thread have some time
            // to release the ring-buffer-slot.
            hilet new_ptr = new Value(std::forward<Args>(args)...);
            hi_assert_not_null(new_ptr);

            _pointer.store(new_ptr, std::memory_order::release);
            return *new_ptr;
        }
    }

    /** Invoke a function on the value if it exists then reset.
     *
     * @note Only one thread should call this function on an object.
     * @param func The function to call with the value as argument if it exists.
     * @return If empty/false the polymorphic_optional was empty, otherwise it contains the return value of the function if any.
     */
    template<typename Func>
    hi_force_inline auto invoke_and_reset(Func&& func) noexcept
    {
        using func_result = decltype(std::declval<Func>()(std::declval<base_type&>()));
        using result_type = std::conditional_t<std::is_same_v<func_result, void>, bool, std::optional<func_result>>;

        // Check if the emplace has finished.
        if (auto ptr = _pointer.load(std::memory_order::acquire)) {
            if constexpr (std::is_same_v<func_result, void>) {
                if (equal_ptr(ptr, this)) {
                    std::forward<Func>(func)(*ptr);
                    std::destroy_at(ptr);

                    // Empty after destroying the value.
                    _pointer.store(nullptr, std::memory_order::release);
                    return true;

                } else {
                    // Since the object is on the heap, we can empty this immediately.
                    _pointer.store(nullptr, std::memory_order::release);

                    std::forward<Func>(func)(*ptr);
                    delete ptr;
                    return true;
                }

            } else {
                if (equal_ptr(ptr, this)) {
                    auto result = std::forward<Func>(func)(*ptr);
                    std::destroy_at(ptr);

                    // Empty after destroying the value.
                    _pointer.store(nullptr, std::memory_order::release);
                    return result_type{std::move(result)};

                } else {
                    // Since the object is on the heap, we can empty this immediately.
                    _pointer.store(nullptr, std::memory_order::release);

                    auto result = std::forward<Func>(func)(*ptr);
                    delete ptr;
                    return result_type{std::move(result)};
                }
            }

        } else {
            return result_type{};
        }
    }

    /** Wait until the optional is empty, emplace a value, then invoke a function on it before committing.
     *
     * @tparam Message The message type derived from value_type to be stored in a free slot.
     * @param func The function to invoke on the message created on the fifo.
     * @param args The arguments passed to the constructor of Message.
     * @return The result of the invoked function.
     */
    template<typename Value, typename Func, typename... Args>
    hi_force_inline auto wait_emplace_and_invoke(Func&& func, Args&&...args) noexcept
    {
        using func_result = decltype(std::declval<Func>()(std::declval<Value&>()));

        if constexpr (sizeof(Value) <= capacity and alignof(Value) <= alignment) {
            // Wait until the _pointer is a nullptr.
            // And acquire the buffer to start overwriting it.
            // There are no other threads that will make this non-null afterwards.
            while (_pointer.load(std::memory_order_acquire)) {
                // If we get here, that would suck, but nothing to do about it.
                [[unlikely]] contended();
            }

            // Overwrite the buffer with the new slot.
            hilet new_ptr = new (_buffer.data()) Value(std::forward<Args>(args)...);
            hi_assume(new_ptr != nullptr);
            hi_axiom(equal_ptr(new_ptr, this));

            if constexpr (std::is_same_v<func_result, void>) {
                // Call the function on the newly created message.
                std::forward<Func>(func)(*new_ptr);

                // Release the buffer for reading.
                _pointer.store(new_ptr, std::memory_order::release);

            } else {
                // Call the function on the newly created message
                auto tmp = std::forward<Func>(func)(*new_ptr);

                // Release the buffer for reading.
                _pointer.store(new_ptr, std::memory_order::release);
                return tmp;
            }

        } else {
            // We need a heap allocated pointer with a fully constructed object
            // Lets do this ahead of time to let another thread have some time
            // to release the ring-buffer-slot.
            hilet new_ptr = new Value(std::forward<Args>(args)...);
            hi_assert_not_null(new_ptr);

            // Wait until the slot.pointer is a nullptr.
            // We don't need to acquire since we wrote into a new heap location.
            // There are no other threads that will make this non-null afterwards.
            while (_pointer.load(std::memory_order::relaxed)) {
                // If we get here, that would suck, but nothing to do about it.
                [[unlikely]] contended();
            }

            if constexpr (std::is_same_v<func_result, void>) {
                // Call the function on the newly created message
                std::forward<Func>(func)(*new_ptr);

                // Release the heap for reading.
                _pointer.store(new_ptr, std::memory_order::release);

            } else {
                // Call the function on the newly created message
                auto tmp = std::forward<Func>(func)(*new_ptr);

                // Release the heap for reading.
                _pointer.store(new_ptr, std::memory_order::release);
                return tmp;
            }
        }
    }

private:
    /** Storage for the object.
     * The buffer is first, so that it matches the alignment of the polymorphic_optional.
     */
    std::array<std::byte, capacity> _buffer;

    /** A pointer to the value.
     * This pointer can have 3 different states:
     * - nullptr: empty
     * - _buffer.data(): object is stored in the buffer.
     * - otherwise: Object is allocated on the heap with `new`.
     */
    std::atomic<pointer> _pointer = nullptr;

    hi_no_inline void contended() noexcept
    {
        using namespace std::chrono_literals;

        // If we get here, that would suck, but nothing to do about it.
        //++global_counter<"polymorphic_optional:contended">;
        std::this_thread::sleep_for(16ms);
    }
};

} // namespace hi::inline v1

hi_warning_pop();
