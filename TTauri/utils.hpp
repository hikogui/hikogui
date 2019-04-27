#pragma once

#include "Logging.hpp"

#include <boost/throw_exception.hpp>

#include <cstdint>
#include <memory>
#include <functional>
#include <algorithm>
#include <thread>
#include <atomic>

namespace TTauri {

struct NotImplementedError : virtual boost::exception, virtual std::exception {};

template<typename T, bool result = std::is_same<decltype(((T *)nullptr)->initialize()), void>::value>
constexpr bool hasInitializeHelper(int)
{
    return result;
}

template<typename T>
constexpr bool hasInitializeHelper(...)
{
    return false;
}

template<typename T>
constexpr bool hasInitialize()
{
    return hasInitializeHelper<T>(0);
}

template<typename T, typename... Args, typename std::enable_if_t<TTauri::hasInitialize<T>(), int> = 0>
inline std::shared_ptr<T> make_shared(Args... args)
{
    auto tmp = std::make_shared<T>(args...);
    tmp->initialize();
    return tmp;
}

template<typename T, typename... Args, typename std::enable_if_t<!TTauri::hasInitialize<T>(), int> = 0>
inline std::shared_ptr<T> make_shared(Args... args)
{
    return std::make_shared<T>(args...);
}

inline constexpr size_t align(size_t offset, size_t alignment) 
{
    return ((offset + alignment - 1) / alignment) * alignment;
}

template<typename T>
inline typename T::value_type pop_back(T &v)
{
    typename T::value_type x = std::move(v.back());
    v.pop_back();
    return x;
}

template<typename T, typename U>
inline std::shared_ptr<T> lock_dynamic_cast(const std::weak_ptr<U> &x)
{
    return std::dynamic_pointer_cast<T>(x.lock());
}

struct GetSharedCastError : virtual boost::exception, virtual std::exception {};

template<typename T, typename std::enable_if_t<std::is_constructible_v<T>, int> = 0>
inline std::shared_ptr<T> get_singleton()
{
    if (!T::singleton) {
        T::singleton = std::make_shared<T>();
    }

    auto tmpCastedShared = std::dynamic_pointer_cast<T>(T::singleton);
    if (!tmpCastedShared) {
        BOOST_THROW_EXCEPTION(GetSharedCastError());
    }

    return tmpCastedShared;
}

template<typename T, typename std::enable_if_t<!std::is_constructible_v<T>, int> = 0>
inline std::shared_ptr<T> get_singleton()
{
    auto tmpCastedShared = std::dynamic_pointer_cast<T>(T::singleton);
    if (!tmpCastedShared) {
        BOOST_THROW_EXCEPTION(GetSharedCastError());
    }

    return tmpCastedShared;
}

struct MakeSharedNotNull : virtual boost::exception, virtual std::exception {};

template<typename T, typename... Args>
inline decltype(auto) make_singleton(Args... args)
{
    if (T::singleton) {
        BOOST_THROW_EXCEPTION(MakeSharedNotNull());
    }

    T::singleton = std::make_shared<T>(args...);
    return T::singleton->initialize();
}

template<typename T>
inline std::enable_if_t<!std::is_pointer_v<T>, T> middle(T begin, T end)
{
    return begin + std::distance(begin, end) / 2;
}

template<typename T>
inline std::enable_if_t<std::is_pointer_v<T>, T> middle(T begin, T end)
{
    return reinterpret_cast<T>((reinterpret_cast<intptr_t>(begin) + reinterpret_cast<intptr_t>(end)) / 2);;
}

template<typename T, typename U>
inline T binary_nearest_find(T begin, T end, U value)
{
    while (begin < end) {
        auto const m = middle(begin, end);

        if (value > *m) {
            begin = m + 1;
        } else if (value < *m) {
            end = m;
        } else {
            return m;
        }
    }
    return begin;
}

template<typename T, typename U, typename F>
inline T transform(const U &input, F operation)
{
    T result = {};
    std::transform(input.begin(), input.end(), std::back_inserter(result), operation);
    return result;
}

template<typename T, size_t N, typename F>
constexpr std::array<T, N> generate_array(F operation)
{
    std::array<T, N> a;

    for (size_t i = 0; i < N; i++) {
        a[i] = operation(i);
    }

    return a;
}

template<typename T>
struct atomic_state {
    struct error : virtual boost::exception, virtual std::exception {};

    std::atomic<T> state;

    atomic_state() = delete;
    virtual ~atomic_state() = default;
    atomic_state(const atomic_state &) = delete;
    atomic_state &operator=(const atomic_state &) = delete;
    atomic_state(atomic_state &&) = delete;
    atomic_state &operator=(atomic_state &&) = delete;

    atomic_state(const T &newState) {
        state = newState;
    }

    bool operator==(T other_state) {
        return state.load() == other_state;
    }

    bool operator!=(T other_state) {
        return state.load() != other_state;
    }

    T value() {
        return state.load();
    }

    /*! Transition between states.
     * \param stateChanges a list of from->to states (in that order).
     * \return Return the original state, or empty when the state could not be changes.
     */
    std::optional<T> try_transition(const std::vector<std::pair<T, T>> &transitions) {
        for (auto const &transition: transitions) {
            auto const from_state = transition.first;
            auto const to_state = transition.second;

            auto expected_state = from_state;
            if (state.compare_exchange_strong(expected_state, to_state)) {
                //LOG_DEBUG("state %i -> %i") % static_cast<int>(from_state) % static_cast<int>(to_state);
                return {from_state};
            }
        }
        return {};
    }

    T transition(const std::vector<std::pair<T, T>> &transitions) {
        for (uint64_t retry = 0; ; retry++) {
            auto const result = try_transition(transitions);
            if (result) {
                return result.value();
            }

            if (retry < 5) {
                // Spin.
            } else if (retry < 50) {
                std::this_thread::yield();
            } else if (retry == 50) {
                LOG_WARNING("atomic_state transition starved.");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }

    T transition_or_throw(const std::vector<std::pair<T, T>> &transitions) {
        auto const result = try_transition(transitions);
        if (!result) {
            BOOST_THROW_EXCEPTION(error());
        }
        return result.value();
    }
};


}
