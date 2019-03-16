#pragma once

#include <boost/throw_exception.hpp>

#include <cstdint>
#include <memory>

namespace TTauri {

inline size_t align(size_t offset, size_t alignment)
{
    return ((offset + alignment - 1) / alignment) * alignment;
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
inline std::shared_ptr<T> makeShared(Args... args)
{
    if (T::shared) {
        BOOST_THROW_EXCEPTION(MakeSharedNotNull());
    }

    auto tmpShared = std::make_shared<T>(args...);
    T::shared = tmpShared;

    tmpShared->initialize();
    return tmpShared;
}

}