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
std::shared_ptr<T> lock_dynamic_cast(const std::weak_ptr<U> &x)
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

    bool set(T newState, const T &oldState) {
        T expected = oldState;
        if (state.compare_exchange_strong(expected, newState)) {
            LOG_DEBUG("state %i -> %i") % static_cast<int>(oldState) % static_cast<int>(newState);
            return true;
        } else {
            return false;
        }
    }

    bool set(T newState, const std::vector<T> &oldStates) {
        for (auto oldState: oldStates) {
            if (set(newState, oldState)) {
                return true;
            }
        }
        return false;
    }

    T setAndWait(const std::vector<std::pair<T, T>> &stateChanges) {
        while (true) {
            for (auto const &stateChange: stateChanges) {
                auto newState = stateChange.first;
                auto oldState = stateChange.second;
                if (set(newState, oldState)) {
                    return oldState;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    T setAndWait(T newState, const std::vector<T> &oldStates) {
        while (true) {
            for (auto oldState: oldStates) {
                if (set(newState, oldState)) {
                    return oldState;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    T setAndWait(T newState, const T &oldState) {
        while (true) {
            if (set(newState, oldState)) {
                return oldState;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void setOrExcept(T newState, const std::vector<T> &oldStates) {
        if (!set(newState, oldStates)) {
            BOOST_THROW_EXCEPTION(Error());
        }
    }

    void setOrExcept(T newState, const T &oldState) {
        if (!set(newState, oldState)) {
            BOOST_THROW_EXCEPTION(Error());
        }
    }
};

}
