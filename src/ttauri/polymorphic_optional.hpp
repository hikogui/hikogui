// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "assert.hpp"
#include "concepts.hpp"
#include <array>
#include <memory>
#include <type_traits>

namespace tt {

/** Polymorphic optional.
 * This optional container can hold an polymorphic value.
 *
 * If the assigned sub-class is larger than the internal buffer
 * the object will be allocated on the heap.
 *
 * @tparam BaseType The base type of the polymorphic value.
 * @tparam Capacity The size in bytes of the internal buffer to store
 *                  the polymorphic value.
 */
template<typename BaseType, size_t Capacity>
class alignas(16) polymorphic_optional {
public:
    using value_type = BaseType;
    using reference = value_type &;
    using const_reference = value_type const &;
    using pointer = value_type *;
    using const_pointer = value_type const *;

    static constexpr size_t capacity = Capacity;

    /** Destroy any contained value.
     */
    ~polymorphic_optional()
    {
        reset();
    }

    polymorphic_optional(polymorphic_optional const &other) = delete;
    polymorphic_optional(polymorphic_optional &&other) = delete;
    polymorphic_optional &operator=(polymorphic_optional const &other) = delete;
    polymorphic_optional &operator=(polymorphic_optional &&other) = delete;

    /** Construct an empty value.
     */
    [[nodiscard]] constexpr polymorphic_optional() noexcept : _state(state::empty) {}

    /** Construct an object as value.
     *
     * @param other An object of a sub-class of the value_type.
     */
    template<typename Other>
    requires(is_different_v<Other, value_type>) [[nodiscard]] polymorphic_optional(Other &&other) noexcept
    {
        static_assert(std::is_base_of_v<value_type, std::remove_cvref_t<Other>>);
        _state = state::empty;
        emplace<std::remove_cvref_t<Other>>(std::forward<Other>(other));
    }

    /** Assign an object.
     *
     * @param other An object of a sub-class of the value_type.
     * @return reference to this.
     */
    template<decayed_derived_from<value_type> Other>
    polymorphic_optional &operator=(Other &&other) noexcept
    {
        emplace<std::remove_cvref_t<Other>>(std::forward<Other>(other));
        return *this;
    }

    /** Construct the contained value in-place.
     *
     * @tparam T The sub-class of value_type to construct.
     * @param args The arguments used to construct the object of type O.
     * @return reference to the constructed object.
     */
    template<derived_from<value_type> T, typename... Args>
    reference emplace(Args &&...args) noexcept
    {
        reset();

        pointer r;
        if (sizeof(T) <= capacity) {
            r = new (_value.buffer.data()) T(std::forward<Args>(args)...);
            _state = state::internal;

        } else {
            r = _value.pointer = new T(std::forward<Args>(args)...);
            _state = state::external;
        }
        tt_axiom(r != nullptr);
        return *r;
    }

    /** Check whether the object contains a value.
     *
     * @return True if the object has a value.
     */
    [[nodiscard]] bool has_value() const noexcept
    {
        return _state != state::empty;
    }

    /** Check whether the object contains a value.
     *
     * @return True if the object has a value.
     */
    [[nodiscard]] operator bool() const noexcept
    {
        return has_value();
    }

    /** Destroys any contained value.
     */
    void reset() noexcept
    {
        switch (_state) {
        case state::internal: std::destroy_at(internal_pointer()); break;
        case state::external: delete external_pointer(); break;
        default:;
        }
        _state = state::empty;
    }

    /** Returns the contained value.
     *
     * @throws std::bad_optional_access when this does not contain a value.
     * @return A reference to the contained value.
     */
    [[nodiscard]] const_reference value() const &noexcept
    {
        if (_state == state::empty) {
            throw std::bad_optional_access();
        }
        return *_pointer();
    }

    /** Returns the contained value.
     *
     * @throws std::bad_optional_access when this does not contain a value.
     * @return A reference to the contained value.
     */
    [[nodiscard]] reference value() &noexcept
    {
        if (_state == state::empty) {
            throw std::bad_optional_access();
        }
        return *_pointer();
    }

    /** Dereference the contained value.
     *
     * It is undefined behaviour to derefence if this does not contain a value.
     *
     * @return A reference to the contained value.
     */
    [[nodiscard]] const_reference operator*() const noexcept
    {
        tt_axiom(_state != state::empty);
        return *_pointer();
    }

    /** Dereference the contained value.
     *
     * It is undefined behaviour to derefence if this does not contain a value.
     *
     * @return A reference to the contained value.
     */
    [[nodiscard]] reference operator*() noexcept
    {
        tt_axiom(_state != state::empty);
        return *_pointer();
    }

    /** Get a pointer to the contained value.
     *
     * @return A pointer to the contained value, or nullptr if this doe not contain a value.
     */
    [[nodiscard]] const_pointer operator->() const noexcept
    {
        return _pointer();
    }

    /** Get a pointer to the contained value.
     *
     * @return A pointer to the contained value, or nullptr if this doe not contain a value.
     */
    [[nodiscard]] pointer operator->() noexcept
    {
        return _pointer();
    }

private:
    enum state { empty, internal, external };

    union {
        std::array<std::byte, capacity> buffer;
        pointer pointer;
    } _value;
    state _state;

    [[nodiscard]] const_pointer internal_pointer() const noexcept
    {
        tt_axiom(_state == state::internal);
        return std::launder(reinterpret_cast<const_pointer>(_value.buffer.data()));
    }

    [[nodiscard]] pointer internal_pointer() noexcept
    {
        tt_axiom(_state == state::internal);
        return std::launder(reinterpret_cast<pointer>(_value.buffer.data()));
    }

    [[nodiscard]] const_pointer external_pointer() const noexcept
    {
        tt_axiom(_state == state::external);
        return std::launder(_value.pointer);
        ;
    }

    [[nodiscard]] pointer external_pointer() noexcept
    {
        tt_axiom(_state == state::external);
        return std::launder(_value.pointer);
        ;
    }

    /** Get a pointer to the contained value.
     *
     * @return A pointer to the contained value, or nullptr if this doe not contain a value.
     */
    [[nodiscard]] const_pointer _pointer() const noexcept
    {
        switch (_state) {
        case state::internal: return internal_pointer();
        case state::external: return external_pointer();
        case state::empty: return nullptr;
        default: tt_no_default();
        }
    }

    /** Get a pointer to the contained value.
     *
     * @return A pointer to the contained value, or nullptr if this doe not contain a value.
     */
    [[nodiscard]] pointer _pointer() noexcept
    {
        switch (_state) {
        case state::internal: return internal_pointer();
        case state::external: return external_pointer();
        case state::empty: return nullptr;
        default: tt_no_default();
        }
    }
};

} // namespace tt
