#pragma once

#include <boost/throw_exception.hpp>

#include <cstdint>
#include <memory>

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

inline size_t align(size_t offset, size_t alignment)
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
}
