// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "utils.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

namespace TTauri {

constexpr char PICKLE_SMALL_NATURAL_MAX = 0x3f;
constexpr char PICKLE_SMALL_STRING_MIN = 0xc0;

constexpr char PICKLE_END_MARK = 0xff;
constexpr char PICKLE_NULL = 0xfe;
constexpr char PICKLE_TRUE = 0xfd;
constexpr char PICKLE_FALSE = 0xfc;
constexpr char PICKLE_STRING = 0xfb;
constexpr char PICKLE_OBJECT = 0xfa;
constexpr char PICKLE_MAP = 0xf9;
constexpr char PICKLE_VECTOR = 0xf8;
constexpr char PICKLE_DOUBLE = 0xf7;
constexpr char PICKLE_GLM_VEC = 0xf6;


struct end_pickle_t {};

inline std::string &operator^(std::string &lhs, end_pickle_t const &rhs)
{
    return lhs;
}

inline std::string &operator^(std::string &lhs, bool rhs)
{
    lhs.push_back(rhs ? PICKLE_TRUE : PICKLE_FALSE);
    return lhs;
}

inline std::string &operator^(std::string &lhs, nullptr_t rhs)
{
    lhs.push_back(PICKLE_NULL);
    return lhs;
}

inline std::string &operator^(std::string &lhs, double rhs)
{
    lhs.push_back(PICKLE_DOUBLE);

    auto u64rhs = bit_cast<uint64_t>(rhs);
    for (size_t i = 0; i < sizeof(u64rhs); i++) {
        lhs.push_back(static_cast<char>(u64rhs & 0xff));
        u64rhs >>= 8;
    }
    return lhs;
}

inline std::string &operator^(std::string &lhs, uint64_t rhs)
{
    while (true) {
        uint8_t const last_value = rhs & 0x7f;
        rhs >>= 7;

        if (rhs == 0 && last_value < 0x40) {
            // rhs is fully shifted in, and the sign-bit is clear.
            // Add a stop bit to mark the last byte.
            lhs.push_back(last_value | 0x80);
            return lhs;
        } else {
            lhs.push_back(last_value);
        }
    }
}

/*! An integer is encoded as a stop-bit encoded as a little endian
 * two's compliment integer.
 *
 * Negative integers are encoded with at least two bytes. This
 * way the codes for 
 */
inline std::string &operator^(std::string &lhs, int64_t rhs)
{
    if (rhs >= 0) {
        return lhs ^ static_cast<uint64_t>(rhs);
    }

    lhs.push_back(rhs & 0x7f);
    rhs >>= 7;

    while (true) {
        uint8_t const last_value = rhs & 0x7f;
        rhs >>= 7;

        if (rhs == -1 && last_value >= 0x40) {
            // rhs is fully shifted in, and the sign-bit is set.
            // Add a stop bit to mark the last byte.
            lhs.push_back(last_value | 0x80);
            return lhs;
        } else {
            lhs.push_back(last_value);
        }
    }
}

inline std::string &operator^(std::string &lhs, int32_t rhs) { return lhs ^ static_cast<int64_t>(rhs); }
inline std::string &operator^(std::string &lhs, int16_t rhs) { return lhs ^ static_cast<int64_t>(rhs); }
inline std::string &operator^(std::string &lhs, int8_t rhs) { return lhs ^ static_cast<int64_t>(rhs); }
inline std::string &operator^(std::string &lhs, uint32_t rhs) { return lhs ^ static_cast<uint64_t>(rhs); }
inline std::string &operator^(std::string &lhs, uint16_t rhs) { return lhs ^ static_cast<uint64_t>(rhs); }
inline std::string &operator^(std::string &lhs, uint8_t rhs) { return lhs ^ static_cast<uint64_t>(rhs); }

inline std::string &operator^(std::string &lhs, void *rhs)
{
    return lhs ^ reinterpret_cast<size_t>(rhs);
}

/*! Pickle a string.
 */
inline std::string &operator^(std::string &lhs, std::string_view const &rhs)
{
    if (rhs.size() <= 0x1f) {
        lhs.push_back(static_cast<uint8_t>(rhs.size()) | PICKLE_SMALL_STRING_MIN);
    } else {
        lhs.push_back(PICKLE_STRING);
        lhs ^ rhs.size();
    }

    lhs += rhs;
    return lhs;
}

inline std::string &operator^(std::string &lhs, char const rhs[]) {
    return lhs ^ std::string_view(rhs);
}


template<int S, typename T, glm::qualifier Q>
inline std::string &operator^(std::string &lhs, glm::vec<S,T,Q> const &rhs)
{
    lhs.push_back(PICKLE_GLM_VEC);

    for (glm::vec<S,T,Q>::length_type i = 0; i < S; i++) {
        lhs ^ rhs[i];
    }

    lhs.push_back(PICKLE_END_MARK);
    return lhs;
}

template<typename T>
inline std::string &operator^(std::string &lhs, std::vector<T> const &rhs)
{
    lhs.push_back(PICKLE_VECTOR);

    for (let &item: rhs) {
        lhs ^ item;
    }

    lhs.push_back(PICKLE_END_MARK);
    return lhs;
}

template<typename K, typename V>
inline std::string &operator^(std::string &lhs, std::map<K,V> const &rhs)
{
    lhs.push_back(PICKLE_MAP);

    for (let &item: rhs) {
        lhs ^ item->first;
        lhs ^ item->second;
    }

    lhs.push_back(PICKLE_END_MARK);
    return lhs;
}

template<typename K, typename V>
inline std::string &operator^(std::string &lhs, std::unordered_map<K,V> const &rhs)
{
    lhs.push_back(PICKLE_MAP);

    for (let &item: rhs) {
        lhs ^ item->first;
        lhs ^ item->second;
    }

    lhs.push_back(PICKLE_END_MARK);
    return lhs;
}

template<typename ...Args>
inline std::string &pickle(std::string &dst, Args&&... args)
{
    dst.clear();
    return (dst ^ ... ^ args);
}


};
