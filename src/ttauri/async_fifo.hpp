
#pragma once

#include "wfree_fifo.hpp"

namespace tt::inline v1 {
namespace detail {

class async_base {
public:
    /** run the async item.
     */
    virtual void run() noexcept;

};

/**
 *
 * We are not using `std::function` for the storage of the functor object
 * as `std::function` may use allocation. Instead we like the functor to
 * live inside the slots of the fifo itself.
 */
template<typename Functor, typename... Arguments>
class async_post : public async_base
{
public:
    static_assert(std::is_invocable_v<Functors, Arguments...>);

    using result_type = void;

    template<typename Func, typename... Args>
    async_send(Func &&functor, Args &&...args) noexcept :
        async_base(), _functor(std::forward<Func>(functor)), _arguments(std::forward<Args>(args)...)

    void run() noexcept override
    {
        std::apply(std::move(_functor), std::move(_arguments));
    }

private:
    Functor _functor;
    std::tuple<Arguments...> _arguments;
};

/**
 *
 * We are not using `std::function` for the storage of the functor object
 * as `std::function` may use allocation. Instead we like the functor to
 * live inside the slots of the fifo itself.
 */
template<typename Functor, typename... Arguments>
class async_send : public async_base
{
public:
    static_assert(std::is_invocable_v<Functors, Arguments...>);

    using result_type = decltype(std::declval<Functor>()(std::declval<Arguments>()...));

    template<typename Func, typename... Args>
    async_send(Func &&functor, Args &&...args) noexcept :
        async_base(), _functor(std::forward<Func>(functor)), _arguments(std::forward<Args>(args)...)

    void run() noexcept override
    {
        try {
            if constexpr (std::is_same_v<result_type,void>) {
                std::apply(std::move(_functor), std::move(_arguments));
                _promise.set_value();
            } else {
                _promise.set_value(std::apply(std::move(_functor), std::move(_arguments)));
            }
        } catch (...) {
            _promise.set_exception(std::current_exception());
        }
    }

    std::future<result_type> get_future() const noexcept
    {
        return _promise.get_future();
    }

private:
    Functor _functor;
    std::promise<result_type> _promise;
    std::tuple<Arguments...> _arguments;
};

}


/** A fifo (First-in, Firts-out) for asynchronous calls.
 *
 * This fifo is used to handle asynchronous calls from an event-loop.
 *
 * @tparam SlotSize The size in bytes of each slot. This determines the maximum
 *                  number of functions that can be stored on the fifo and if
 *                  functions can be completely stored on the fifo or are allocated on the heap.
 */
template<std::size_t SlotSize = 64>
class async_fifo {
public:
    constexpr async_fifo() noexcept = default;
    async_fifo(async_fifo const &) = delete;
    async_fifo(async_fifo &&) = delete;
    async_fifo &operator=(async_fifo const &) = delete;
    async_fifo &operator=(async_fifo &&) = delete;

    /** Run one of the function that was posted or send.
     *
     * @retval true One async function has been taken from the fifo and run.
     * @retval false The fifo was empty.
     */
    bool run_one() noexcept
    {
        return _fifo.take_one([](auto &item) {
            item.run();
        });
    }

    /** Run all the functions posted or send on the fifo.
     *
     * This function calls `run_one()` until the fifo is empty.
     */
    void run_all() noexcept
    {
        while (run_one()) {}
    }

    /** Asynchronously send a functor to the fifo to be executed later.
     *
     * The function object and arguments are stored within the fifo and does
     * not need allocation. However the `std::promise` object will allocate the
     * return object on the heap as it must be shared with `std::future`.
     *
     * @param func A function object.
     * @param args The arguments to pass to the function when called.
     * @return A `std::future` with the result of `func`. The result type may be `void`.
     */
    template<typename Func, typename... Args>
    auto send(Func &&func, Args &&... args) noexcept
        requires(std::is_invocable_v<std::decay_t<Func>, std::decay_t<Args>...>)
    {
        using async_type = detail::async_send<std::decay_t<Func>, std::decay_t<Args>...>;

        ttlet &item = _fifo.emplace<async_type>(std::forward<Func>(func), std::forward<Args>(args)...);
        return item.get_future();
    }

    /** Asynchronously post a functor to the fifo to be executed later.
     *
     * The function object and arguments are stored within the fifo and does
     * not need allocation.
     *
     * @note wait-free if the function object and arguments fit in the message slot of the fifo.
     * @param func A function object.
     * @param args The arguments to pass to the function when called.
     */
    template<typename Func, typename... Args>
    void post(Func &&func, Args &&... args) noexcept
        requires(std::is_invocable_v<std::decay_t<Func>, std::decay_t<Args>...>)
    {
        using async_type = detail::async_post<std::decay_t<Func>, std::decay_t<Args>...>;

        _fifo.emplace<async_type>(std::forward<Func>(func), std::forward<Args>(args)...);
    }

private:
    wfree_fifo<detail::async_base, SlotSize> _fifo;
}


}

