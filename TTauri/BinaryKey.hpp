// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <string>
#include <typeinfo>
#include <typeindex>
#include <glm/glm.hpp>

namespace TTauri {

template<typename T>
inline size_t binarykey_typeindex(T const &value)
{
    return std::type_index(typeid(value)).hash_code();
}

template<> inline size_t binarykey_typeindex(int8_t const& value) { return 1; }
template<> inline size_t binarykey_typeindex(uint8_t const& value) { return 2; }
template<> inline size_t binarykey_typeindex(int16_t const& value) { return 3; }
template<> inline size_t binarykey_typeindex(uint16_t const& value) { return 4; }
template<> inline size_t binarykey_typeindex(int32_t const& value) { return 5; }
template<> inline size_t binarykey_typeindex(uint32_t const& value) { return 6; }
template<> inline size_t binarykey_typeindex(int64_t const& value) { return 7; }
template<> inline size_t binarykey_typeindex(uint64_t const& value) { return 8; }
template<> inline size_t binarykey_typeindex(float const& value) { return 9; }
template<> inline size_t binarykey_typeindex(double const& value) { return 10; }
template<> inline size_t binarykey_typeindex(glm::vec2 const& value) { return 11; }
template<> inline size_t binarykey_typeindex(std::string const& value) { return 12; }
template<> inline size_t binarykey_typeindex(char const * const& value) { return 13; }

template<typename T>
inline size_t binarykey_typeindexsize(T const &value)
{
    let typeIndex = binarykey_typeindex(value);
    return typeIndex <= 127 ? sizeof(char): sizeof(typeIndex);
}

template<typename T>
inline void binarykey_append_type(std::string& data, T const &value)
{
    let typeIndex = binarykey_typeindex(value);

    if (typeIndex <= 127) {
        data.append(1, static_cast<char>(typeIndex));
    } else {
        data.append(reinterpret_cast<const char*>(&typeIndex), sizeof(typeIndex));
    }
}

template<typename T>
inline void binarykey_append_value(std::string &data, T const &value)
{
    data.append(reinterpret_cast<const char *>(&value), sizeof(value));
}

template<>
inline void binarykey_append_value(std::string &data, std::string const &value)
{
    data.append(value);
}

template<>
inline void binarykey_append_value(std::string& data, char const * const& value)
{
    data.append(value);
}

inline void binarykey_append_typevalues(std::string &data) {}

template<typename T, typename... Targs>
inline void binarykey_append_typevalues(std::string &data, T value, Targs... Fargs)
{
    binarykey_append_type(data, value);
    binarykey_append_value(data, value);
    binarykey_append_typevalues(data, Fargs...);
}

template<typename T>
inline size_t binarykey_size(T const& value)
{
    return binarykey_typeindexsize(value) + sizeof(value);
}

template<>
inline size_t binarykey_size(std::string const& value) {
    return binarykey_typeindexsize(value) + value.size();
}

inline size_t binarykey_size(char const * const &value) {
    return binarykey_typeindexsize(value) + strlen(value);
}

inline size_t binarykey_total_size()
{
    return 0;
}

template<typename T, typename... Targs>
inline size_t binarykey_total_size(T value, Targs... Fargs)
{
    return binarykey_size(value) + binarykey_total_size(Fargs...);
}

template<typename... Targs>
inline std::string binarykey_create_data(Targs... Fargs)
{
    std::string data;
    data.reserve(binarykey_total_size(Fargs...));
    binarykey_append_typevalues(data, Fargs...);
    return data;
}

struct BinaryKey {
    std::string data;

    template<typename... Targs>
    BinaryKey(Targs... Fargs) : data(binarykey_create_data(Fargs...)) {}

    BinaryKey(BinaryKey const &other) : data(other.data) {}
    BinaryKey(BinaryKey &&other) : data(std::move(other.data)) {}

    template<typename... Targs>
    void update(Targs... Fargs) {
        data.clear();
        binarykey_append_typevalues(data, Fargs...);
    }

    bool operator==(BinaryKey const &other) const {
        return data == other.data;
    }
};

}

namespace std {

template<>
struct hash<TTauri::BinaryKey> {
    size_t operator()(const TTauri::BinaryKey &key) const {
        return std::hash<std::string>{}(key.data);
    }
};

}
