// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "wfree_fifo.hpp"
#include <concepts>

namespace tt::inline v1 {
namespace detail {

class event_queue_item_base {
public:
    virtual ~event_queue_item_base() = default;
    virtual void operator()() const noexcept = 0;
};

template<std::invocable Function>
class event_queue_item final : public event_queue_item_base {
public:
    constexpr event_queue_item(event_queue_item const &) noexcept = default;
    constexpr event_queue_item(event_queue_item &&) noexcept = default;
    constexpr event_queue_item &operator=(event_queue_item const &) noexcept = default;
    constexpr event_queue_item &operator=(event_queue_item &&) noexcept = default;

    constexpr event_queue_item(Function const &function) noexcept : function(function) {}
    constexpr event_queue_item(Function &&function) noexcept : function(std::move(function)) {}

    virtual void operator()() const noexcept
    {
        function();
    }

private:
    Function function;
};
} // namespace detail

class event_queue {
public:
    void emplace(std::invocable auto &&function) const noexcept
    {
        using function_type = std::remove_cvref_t<decltype(function)>;

        return fifo.emplace<detail::event_queue_item<function_type>>(std::forward<decltype(function)>(function));
    }

    bool take_one(std::invocable<detail::event_queue_item_base const &> auto &&operation) const noexcept
    {
        return fifo.take_one(std::forward<decltype(operation)>(operation));
    }

    void take_all(std::invocable<detail::event_queue_item_base const &> auto &&operation) const noexcept
    {
        return fifo.take_all(std::forward<decltype(operation)>(operation));
    }

private:
    mutable wfree_fifo<detail::event_queue_item_base, 128> fifo;
};

} // namespace tt
