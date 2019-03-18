#pragma once

#include <boost/throw_exception.hpp>

#include <cstdint>
#include <memory>

namespace TTauri {

inline size_t align(size_t offset, size_t alignment)
{
    return ((offset + alignment - 1) / alignment) * alignment;
}

struct CheckedDynamicCastError : virtual boost::exception, virtual std::exception {};

template<typename T, typename U>
T checked_dynamic_cast(U x)
{
    T xCasted = dynamic_cast<T>(x);
    if (!xCasted) {
        BOOST_THROW_EXCEPTION(CheckedDynamicCastError());
    }
    return xCasted;
}

template<typename T, typename U>
std::shared_ptr<T> lock_dynamic_cast(const std::weak_ptr<U> &x)
{
    auto xLocked = x.lock();
    return std::dynamic_pointer_cast<T>(xLocked);
}

struct CheckedLockDynamicCastError : virtual boost::exception, virtual std::exception {};

template<typename T, typename U>
std::shared_ptr<T> checked_lock_dynamic_cast(const std::weak_ptr<U> &x)
{
    auto xCasted = lock_dynamic_cast(x);
    if (!xCasted)
        BOOST_THROW_EXCEPTION(CheckedLockDynamicCastError());
    }
    return xCasted;
}

struct GetSharedNull : virtual boost::exception, virtual std::exception {};
struct GetSharedCastError : virtual boost::exception, virtual std::exception {};

template<typename T>
inline std::shared_ptr<T> getShared()
{
    auto tmpShared = T::shared;

    if (!tmpShared) {
        BOOST_THROW_EXCEPTION(GetSharedNull());
    }

    auto tmpCastedShared = std::dynamic_pointer_cast<T>(tmpShared);
    if (!tmpCastedShared) {
        BOOST_THROW_EXCEPTION(GetSharedCastError());
    }

    return tmpCastedShared;
}

struct MakeSharedNotNull : virtual boost::exception, virtual std::exception {};

template<typename T, typename... Args>
decltype(auto) makeShared(Args... args)
{
    if (T::shared) {
        BOOST_THROW_EXCEPTION(MakeSharedNotNull());
    }

    auto tmpShared = std::make_shared<T>(args...);
    T::shared = tmpShared;

    return tmpShared->initialize();
}
}
