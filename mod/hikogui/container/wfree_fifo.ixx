// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <type_traits>
#include <concepts>
#include <atomic>
#include <memory>
#include <array>
#include <bit>
#include <chrono>
#include <format>

export module hikogui_container_wfree_fifo;
import hikogui_container_polymorphic_optional;
import hikogui_utility;

hi_warning_push();
// C26490: Don't use reinterpret_cast (type.1).
// Implementing a container.
hi_warning_ignore_msvc(26490);

export namespace hi::inline v1 {

/** A wait-free multiple-producer/single-consumer fifo designed for absolute performance.
 * Because of performance reasons the ring-buffer is 64kByte.
 * Each slot in the ring buffer consists of a pointer and a byte buffer for storage.
 *
 * The number of slots in the ring-buffer is dictated by the size of each
 * slot and the ring-buffer size.
 *
 * @tparam T Base class of the value type stored in the ring buffer.
 * @tparam SlotSize Size of each slot, must be power-of-two.
 */
template<typename T, std::size_t SlotSize>
class alignas(SlotSize) wfree_fifo {
public:
    static_assert(std::has_single_bit(SlotSize), "Only power-of-two number of messages size allowed.");
    static_assert(SlotSize < 65536);

    using value_type = T;
    using slot_type = polymorphic_optional<value_type, SlotSize, SlotSize>;

    constexpr static std::size_t fifo_size = 65536;
    constexpr static std::size_t slot_size = SlotSize;
    constexpr static std::size_t num_slots = fifo_size / slot_size;

    constexpr wfree_fifo() noexcept = default;
    wfree_fifo(wfree_fifo const&) = delete;
    wfree_fifo(wfree_fifo&&) = delete;
    wfree_fifo& operator=(wfree_fifo const&) = delete;
    wfree_fifo& operator=(wfree_fifo&&) = delete;

    /** Check if fifo is empty.
     *
     * @note Must be called on the reader-thread.
     */
    [[nodiscard]] bool empty() const noexcept
    {
        return _head.load(std::memory_order::relaxed) == _tail;
    }

    /** Take one message from the fifo slot.
     * Reads one message from the ring buffer and passes it to a call of operation.
     * If no message is available this function returns without calling operation.
     *
     * @param func The function to call with the value as argument if it exists.
     * @return If empty/false the this was empty, otherwise it contains the return value of the function if any.
     */
    template<typename Func>
    auto take_one(Func&& func) noexcept
    {
        auto result = get_slot(_tail).invoke_and_reset(std::forward<Func>(func));
        if (result) {
            _tail += slot_size;
        }
        return result;
    }

    /** Take all message from the queue.
     * Reads each message from the ring buffer and passes it to a call of operation.
     * If no message are available this function returns without calling operation.
     *
     * @param operation A `void(value_type const &)` which is called when a message is available.
     */
    template<typename Operation>
    void take_all(Operation const& operation) noexcept
    {
        while (take_one(operation)) {}
    }

    /** Create an message in-place on the fifo.
     *
     * @tparam Message The message type derived from value_type to be stored in a free slot.
     * @param func The function to invoke on the message created on the fifo.
     * @param args The arguments passed to the constructor of Message.
     * @return A reference to the emplaced message.
     */
    template<typename Message, typename Func, typename... Args>
    hi_force_inline auto emplace_and_invoke(Func&& func, Args&&...args) noexcept
    {
        // We need a new offset.
        // - The offset is a byte index into 64kByte of memory.
        // - Increment offset by the slot_size and the _head will overflow naturally
        //   at the end of the fifo.
        // - We don't care about memory ordering with other writer threads. as
        //   each slot has an atomic for handling read/writer contention.
        // - We don't have to check full/empty, this is done on the slot itself.
        hilet offset = _head.fetch_add(slot_size, std::memory_order::relaxed);
        return get_slot(offset).template wait_emplace_and_invoke<Message>(std::forward<Func>(func), std::forward<Args>(args)...);
    }

    template<typename Func, typename Object>
    hi_force_inline auto insert_and_invoke(Func&& func, Object&& object) noexcept
    {
        return emplace_and_invoke<std::decay_t<Object>>(std::forward<Func>(func), std::forward<Object>(object));
    }

    template<typename Message, typename... Args>
    hi_force_inline void emplace(Args&&...args) noexcept
    {
        return emplace_and_invoke<Message>([](Message&) -> void {}, std::forward<Args>(args)...);
    }

    template<typename Object>
    hi_force_inline void insert(Object &&object) noexcept
    {
        return emplace<std::decay_t<Object>>(std::forward<Object>(object));
    }

private:
    std::array<slot_type, num_slots> _slots = {}; // must be at offset 0
    std::atomic<uint16_t> _head = 0;
    std::array<std::byte, hi::hardware_destructive_interference_size> _dummy = {};
    uint16_t _tail = 0;

    /** Get the slot that either the _head or _tail are pointing at.
     */
    hi_force_inline slot_type& get_slot(uint16_t offset) noexcept
    {
        hi_axiom(offset % slot_size == 0);
        // The head and tail are 16 bit offsets within the _slots, which are
        return *std::launder(
            std::assume_aligned<slot_size>(reinterpret_cast<slot_type *>(reinterpret_cast<char *>(this) + offset)));
    }
};

} // namespace hi::inline v1

hi_warning_pop();
