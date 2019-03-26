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

template<typename T, typename... Args, typename std::enable_if<TTauri::hasInitialize<T>(), int>::type = 0>
inline std::shared_ptr<T> make_shared(Args... args)
{
    auto tmp = std::make_shared<T>(args...);
    tmp->initialize();
    return tmp;
}

template<typename T, typename... Args, typename std::enable_if<!TTauri::hasInitialize<T>(), int>::type = 0>
inline std::shared_ptr<T> make_shared(Args... args)
{
    return std::make_shared<T>(args...);
}

inline constexpr size_t align(size_t offset, size_t alignment) 
{
    return ((offset + alignment - 1) / alignment) * alignment;
}

template<typename T, typename U>
inline std::shared_ptr<T> lock_dynamic_cast(const std::weak_ptr<U> &x)
{
    return std::dynamic_pointer_cast<T>(x.lock());
}

struct GetSharedCastError : virtual boost::exception, virtual std::exception {};

template<typename T>
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

template<typename T, typename U>
inline T transform(const U &input, const std::function<typename T::value_type(const typename U::value_type &)> &operation)
{
    T result = {};
    std::transform(input.begin(), input.end(), std::back_inserter(result), operation);
    return result;
}

template<typename T>
struct atomic_state {
    using value_type = T;

    struct Error : virtual boost::exception, virtual std::exception {};

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

    /*! Transition between states.
     * \param stateChanges a list of from->to states (in that order).
     * \return Return the original state, or empty when the state could not be changes.
     */
    std::optional<T> transition(const std::vector<std::pair<T, T>> &stateChanges) {
        for (auto const &stateChange: stateChanges) {
            auto const fromState = stateChanges.first;
            auto const toState = stateChanges.second;

            T expectedState = fromState;
            if (state.compare_exchange_strong(expectedState, toState)) {
                LOG_DEBUG("state %i -> %i") % static_cast<int>(fromState) % static_cast<int>(toState);
                return {fromState};
            }
        }
        return {};
    }

    T transitionOrWait(const std::vector<std::pair<T, T>> &stateChanges) {
        for (uint64_t retry = 0; ; retry++) {
            auto const result = transition(stateChanges);
            if (result) {
                return result.value();
            }

            if (retry < 5) {
                // Spin.
            } else if (retry < 50) {
                std::this_thread::yield();
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }

    T transitionOrExcept(const std::vector<std::pair<T, T>> &stateChanges) {
        auto const result = transition(stateChanges);
        if (!result) {
            BOOST_THROW_EXCEPTION(Error());
        }
        return result;
    }
};

template <typename T>
struct scoped_state_stransition {
    using state_change = std::pair<T::value_type, T::value_type>;
    using state_changes = std::vector<state_change>;

    T *state;

    state_changes inputStateChanges;
    state_changes exitStateState;

    scoped_state_stransition() = delete;
    virtual ~atomic_state() = default;
    scoped_state_stransition(const scoped_state_stransition &) = delete;
    scoped_state_stransition &operator=(const scoped_state_stransition &) = delete;
    scoped_state_stransition(scoped_state_stransition &&) = delete;
    scoped_state_stransition &operator=(scoped_state_stransition &&) = delete;

    static const state_chages createExitStateChanges(const state_chages &inputStateChanges, T::value_type exitState)
    {
        return transform<state_changes>(inputStateChanges, [&exitState](const state_change &x) { return {x.second, exitState}; }
    }

    scoped_state_stransition(T &state, const state_chages &inputStateChanges, const state_chages &exitStateChanges = {}) :
        state(&state),
        inputStateChanges(exitStateChanges.size() > 0 ? {} : inputStateChanges),
        exitStateState(exitStateState)
    {

    }

    scoped_state_stransition(T &state, const state_changes &intputStateChanges, T::value_type exitState) :
        scoped_state_stransition(state, inputStateChanges, scoped_state_stransition::createExitStateChanges(inputStateChanges, exitState)) {}

}



}
