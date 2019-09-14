// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Diagnostic/exceptions.hpp"
#include "TTauri/Required/URL.hpp"
#include "TTauri/Required/memory.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

namespace TTauri {

enum class pickle_type_t {
    EndMark,
    Null,
    Boolean,
    Integer,
    String,
    Object,
    Map,
    Vector,
    Double,
    GLMVec,
    URL,
    Reserved
};

constexpr uint8_t PICKLE_SMALL_NATURAL_MIN = 0x00;
constexpr uint8_t PICKLE_SMALL_NATURAL_MAX = 0x3f;

constexpr uint8_t PICKLE_SMALL_STRING_MIN = 0xc0;
constexpr uint8_t PICKLE_SMALL_STRING_MAX = 0xdf;

constexpr uint8_t PICKLE_END_MARK = 0xff;
constexpr uint8_t PICKLE_NULL = 0xfe;
constexpr uint8_t PICKLE_TRUE = 0xfd;
constexpr uint8_t PICKLE_FALSE = 0xfc;
constexpr uint8_t PICKLE_STRING = 0xfb;
constexpr uint8_t PICKLE_OBJECT = 0xfa;
constexpr uint8_t PICKLE_MAP = 0xf9;
constexpr uint8_t PICKLE_VECTOR = 0xf8;
constexpr uint8_t PICKLE_DOUBLE = 0xf7;
constexpr uint8_t PICKLE_GLM_VEC = 0xf6;
constexpr uint8_t PICKLE_URL = 0xf5;

constexpr uint8_t PICKLE_RESERVED_MIN = 0x40;
constexpr uint8_t PICKLE_RESERVED_MAX = 0xf4;

template<typename Iter>
inline pickle_type_t pickle_type(Iter &i, Iter const &end)
{
    if (i == end) {
        TTAURI_THROW(parse_error("End of stream"));
    }

    switch (let c_ = static_cast<uint8_t>(*i)) {
    case PICKLE_END_MARK: return pickle_type_t::EndMark;
    case PICKLE_NULL: return pickle_type_t::Null;
    case PICKLE_TRUE: return pickle_type_t::Boolean;
    case PICKLE_FALSE: return pickle_type_t::Boolean;
    case PICKLE_STRING: return pickle_type_t::String;
    case PICKLE_OBJECT: return pickle_type_t::Object;
    case PICKLE_MAP: return pickle_type_t::Map;
    case PICKLE_VECTOR: return pickle_type_t::Vector;
    case PICKLE_DOUBLE: return pickle_type_t::Double;
    case PICKLE_GLM_VEC: return pickle_type_t::GLMVec;
    case PICKLE_URL: return pickle_type_t::URL;
    default:
        if (c_ >= PICKLE_SMALL_STRING_MIN && c_ <= PICKLE_SMALL_STRING_MAX) {
            return pickle_type_t::String;
        } else if (c_ >= PICKLE_RESERVED_MIN && c_ <= PICKLE_RESERVED_MAX) {
            return pickle_type_t::Reserved;
        } else {
            return pickle_type_t::Integer;
        }
    }
}

template<typename R, typename Iter>
inline R unpickle(Iter &&i, Iter &&end)
{
    not_implemented;
}

template<typename Iter>
inline int64_t unpickle(Iter &i, Iter const &end)
{
    // Type conversions.
    switch (pickle_type(i, end)) {
    case pickle_type_t::Null:
        i++;
        return 0;

    case pickle_type_t::Boolean:
        return (static_cast<uint8_t>(*(i++)) == PICKLE_TRUE) ? 1 : 0;

    case pickle_type_t::Double:
        return numeric_cast<int64_t>(unpickle<double>(i, end));

    case pickle_type_t::Integer:
        goto impl;

    default:
        TTAURI_THROW(parse_error("Unexpected type in stream."));
    }

impl:
    size_t nr_bits = 0;
    uint64_t u64value = 0;
    do {
        uint8_t c = *(i++);

        nr_bits += 7;
        u64value <<= 7;
        u64value |= (c & 0x7f);

        // Continue until stop-bit is set.
    } while (c & 0x80);

    // Sign extent by left shifting the unsigned value to align to the
    // MSB. Then converting to an signed value and shift right (which does
    // sign extending).
    required_assert(nr_bits < 64);
    size_t sign_extent_shift = 64 - nr_bits;
    u64value <<= sign_extent_shift;

    auto i64value = bit_cast<int64_t>(u64value);
    return i64value >> sign_extent_shift;
}

template<typename Iter>
inline uint64_t unpickle(Iter &i, Iter const &end)
{
    // Type conversions.
    switch (pickle_type(i, end)) {
    case pickle_type_t::Null:
        i++;
        return 0;

    case pickle_type_t::Boolean:
        return (static_cast<uint8_t>(*(i++)) == PICKLE_TRUE) ? 1 : 0;

    case pickle_type_t::Double:
        return numeric_cast<uint64_t>(unpickle<double>(i, end));

    case pickle_type_t::Integer:
        goto impl;

    default:
        TTAURI_THROW(parse_error("Unexpected type in stream."));
    }

impl:
    size_t nr_bits = 0;
    uint64_t u64value = 0;
    do {
        uint8_t c = *(i++);

        nr_bits += 7;
        u64value <<= 7;
        u64value |= (c & 0x7f);

        // Continue until stop-bit is set.
    } while (c & 0x80);
    return u64value;
}

template<typename Iter>
inline double unpickle(Iter &i, Iter const &end)
{
    // Type conversions.
    switch (pickle_type(i, end)) {
    case pickle_type_t::Null:
        i++;
        return 0.0;

    case pickle_type_t::Boolean:
        return (static_cast<uint8_t>(*(i++)) == PICKLE_TRUE) ? 1.0 : 0.0;

    case pickle_type_t::Double:
        goto impl;

    case pickle_type_t::Integer:
        return numeric_cast<double>(unpickle<int64_t>(i, end));

    default:
        TTAURI_THROW(parse_error("Unexpected type in stream."));
    }

impl:
    i++;

    if ((end - i) < sizeof(u64value)) {
        TTAURI_THROW(parse_error("End of stream"));
    }

    uint64_t u64value = 0;
    for (size_t 0; i < sizeof(u64value); i++) {
        u64value <<= 8;
        u64value |= static_cast<uint8_t>(*(i++));
    }

    return bit_cast<double>(u64value);
}

template<typename Iter> inline uint32_t unpickle(Iter &i, Iter const &end) { return numeric_cast<int32_t>(unpickle<uint64_t>(i, end)); }
template<typename Iter> inline uint16_t unpickle(Iter &i, Iter const &end) { return numeric_cast<int16_t>(unpickle<uint64_t>(i, end)); }
template<typename Iter> inline uint8_t unpickle(Iter &i, Iter const &end) { return numeric_cast<int8_t>(unpickle<uint64_t>(i, end)); }
template<typename Iter> inline int32_t unpickle(Iter &i, Iter const &end) { return numeric_cast<int32_t>(unpickle<int64_t>(i, end)); }
template<typename Iter> inline int16_t unpickle(Iter &i, Iter const &end) { return numeric_cast<int16_t>(unpickle<int64_t>(i, end)); }
template<typename Iter> inline int8_t unpickle(Iter &i, Iter const &end) { return numeric_cast<int8_t>(unpickle<int64_t>(i, end)); }
template<typename Iter> inline float unpickle(Iter &i, Iter const &end) { return numeric_cast<float>(unpickle<double>(i, end)); }

template<typename Iter>
inline std::string unpickle(Iter &i, Iter const &end)
{
    // Type conversions.
    switch (pickle_type(i, end)) {
    case pickle_type_t::String:
        goto impl;
    case pickle_type_t::URL:
        goto impl;
    default:
        TTAURI_THROW(parse_error("Unexpected type in stream."));
    }

impl:
    size_t stringLength = 0;
    if (static_cast<uint8_t>(*i) == PICKLE_STRING || static_cast<uint8_t>(*i) == PICKLE_URL) {
        i++;
        stringLength = unpickle<size_t>(i, begin);
    } else {
        stringLength = c - PICKLE_SMALL_STRING_MIN;
    }

    let beginOfString = i;
    let endOfString = beginOfString + stringLength;
    if (end - i < stringLength) {
        TTAURI_THROW(parse_error("End of stream"));
    }

    i = endOfString;
    return {beginOfString, endOfString};
}

template<typename Iter>
inline URL unpickle(Iter &i, Iter const &end)
{
    return URL{unpickle<std::string>(i, end)};
}

template<typename Iter>
inline bool unpickle(Iter &i, Iter const &end)
{
    switch (pickle_type(i, end)) {
    case pickle_type_t::Null:
        i++;
        return false;

    case pickle_type_t::Boolean:
        return static_cast<uint8_t>(*(i++)) == PICKLE_TRUE;

    case pickle_type_t::Double:
        return unpickle<double>(i, end) > 0.0;

    case pickle_type_t::Integer:
        return unpickle<int64_t>(i, end) > 0;

    default:
        TTAURI_THROW(parse_error("Unexpected type in stream."));
    }
}

template<typename T, typename Iter>
inline std::vector<T> unpickle(Iter &i, Iter const &end)
{
    switch (pickle_type(i, end)) {
    case pickle_type_t::Vector: {
        i++; // Skip over vector-opcode.

        std::vector<T> r;
        while (pickle_type(i, end) != pickle_type_t::EndMark) {
            r.push_back(unpickle<T>(i, end));
        }

        i++; // Skip over end-mark.
        return r;
        }

    default:
        TTAURI_THROW(parse_error("Unexpected type in stream."));
    }
}

template<typename K, typename T, typename Iter>
inline std::map<K,T> unpickle(Iter &i, Iter const &end)
{
    switch (pickle_type(i, end)) {
    case pickle_type_t::Vector: {
        i++; // Skip over vector-opcode.

        std::map<K,T> r;
        while (pickle_type(i, end) != pickle_type_t::EndMark) {
            r.emplace(unpickle<K>(i, end), unpickle<T>(i, end));
        }

        i++; // Skip over end-mark.
        return r;
    }

    default:
        TTAURI_THROW(parse_error("Unexpected type in stream."));
    }
}

template<typename K, typename T, typename Iter>
inline std::unordered_map<K,T> unpickle(Iter &i, Iter const &end)
{
    switch (pickle_type(i, end)) {
    case pickle_type_t::Vector: {
        i++; // Skip over vector-opcode.

        std::unordered_map<K,T> r;
        while (pickle_type(i, end) != pickle_type_t::EndMark) {
            r.emplace(unpickle<K>(i, end), unpickle<T>(i, end));
        }

        i++; // Skip over end-mark.
        return r;
    }

    default:
        TTAURI_THROW(parse_error("Unexpected type in stream."));
    }
}

template<typename R>
inline R unpickle(std::string const &stream)
{
    return unpickle<R>(stream.begin(), stream.end());
}

inline void pickleAppend(std::string &lhs, bool rhs) noexcept
{
    lhs.push_back(rhs ? PICKLE_TRUE : PICKLE_FALSE);
}

inline void pickleAppend(std::string &lhs, nullptr_t rhs) noexcept
{
    lhs.push_back(PICKLE_NULL);
}

inline void pickleAppend(std::string &lhs, double rhs) noexcept
{
    lhs.push_back(PICKLE_DOUBLE);

    auto u64rhs = bit_cast<uint64_t>(rhs);
    for (size_t i = 0; i < sizeof(u64rhs); i++) {
        lhs.push_back(static_cast<char>(u64rhs & 0xff));
        u64rhs >>= 8;
    }
}

inline void pickleAppend(std::string &lhs, uint64_t rhs) noexcept
{
    while (true) {
        uint8_t const last_value = rhs & 0x7f;
        rhs >>= 7;

        if (rhs == 0 && last_value < 0x40) {
            // rhs is fully shifted in, and the sign-bit is clear.
            // Add a stop bit to mark the last byte.
            lhs.push_back(last_value | 0x80);
            return;
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
inline void pickleAppend(std::string &lhs, int64_t rhs) noexcept
{
    if (rhs >= 0) {
        return pickleAppend(lhs, static_cast<uint64_t>(rhs));
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
            return;
        } else {
            lhs.push_back(last_value);
        }
    }
}

inline void pickleAppend(std::string &lhs, int32_t rhs) noexcept { return pickleAppend(lhs, static_cast<int64_t>(rhs)); }
inline void pickleAppend(std::string &lhs, int16_t rhs) noexcept { return pickleAppend(lhs, static_cast<int64_t>(rhs)); }
inline void pickleAppend(std::string &lhs, int8_t rhs) noexcept { return pickleAppend(lhs, static_cast<int64_t>(rhs)); }
inline void pickleAppend(std::string &lhs, uint32_t rhs) noexcept { return pickleAppend(lhs, static_cast<uint64_t>(rhs)); }
inline void pickleAppend(std::string &lhs, uint16_t rhs) noexcept { return pickleAppend(lhs, static_cast<uint64_t>(rhs)); }
inline void pickleAppend(std::string &lhs, uint8_t rhs) noexcept { return pickleAppend(lhs, static_cast<uint64_t>(rhs)); }
inline void pickleAppend(std::string &lhs, void *rhs) noexcept { return pickleAppend(lhs, reinterpret_cast<size_t>(rhs)); }

inline void pickleAppend(std::string &lhs, URL const &rhs) noexcept
{
    auto s = to_string(rhs);

    lhs.push_back(PICKLE_URL);
    pickleAppend(lhs, s.size());
    lhs += s;
}

/*! Pickle a string.
 */
inline void pickleAppend(std::string &lhs, std::string_view const &rhs) noexcept
{
    if (rhs.size() <= 0x1f) {
        lhs.push_back(static_cast<uint8_t>(rhs.size()) | PICKLE_SMALL_STRING_MIN);
    } else {
        lhs.push_back(PICKLE_STRING);
        pickleAppend(lhs, rhs.size());
    }

    lhs += rhs;
}

inline void pickleAppend(std::string &lhs, char const rhs[]) noexcept
{
    return pickleAppend(lhs, std::string_view(rhs));
}

inline void pickleAppend(std::string &lhs, std::string const &rhs) noexcept
{
    return pickleAppend(lhs, std::string_view(rhs));
}

template<int S, typename T, glm::qualifier Q>
inline void pickleAppend(std::string &lhs, glm::vec<S,T,Q> const &rhs) noexcept
{
    lhs.push_back(PICKLE_GLM_VEC);

    for (typename glm::vec<S,T,Q>::length_type i = 0; i < S; i++) {
        pickleAppend(lhs, rhs[i]);
    }

    lhs.push_back(PICKLE_END_MARK);
}

template<typename T>
inline void pickleAppend(std::string &lhs, std::vector<T> const &rhs) noexcept
{
    lhs.push_back(PICKLE_VECTOR);

    for (let &item: rhs) {
        pickleAppend(lhs, item);
    }

    lhs.push_back(PICKLE_END_MARK);
}

template<typename K, typename V>
inline void pickleAppend(std::string &lhs, std::map<K,V> const &rhs) noexcept
{
    lhs.push_back(PICKLE_MAP);

    for (let &item: rhs) {
        pickleAppend(lhs, item->first);
        pickleAppend(lhs, item->second);
    }

    lhs.push_back(PICKLE_END_MARK);
}

template<typename K, typename V>
inline void pickleAppend(std::string &lhs, std::unordered_map<K,V> const &rhs) noexcept
{
    lhs.push_back(PICKLE_MAP);

    for (let &item: rhs) {
        pickleAppend(lhs, item->first);
        pickleAppend(lhs, item->second);
    }

    lhs.push_back(PICKLE_END_MARK);
}

template<typename T, typename U, typename... Args>
inline void pickleAppend(std::string& dst, T&& firstArg, U&& secondArg, Args&&... args) noexcept
{
    pickleAppend(dst, firstArg);
    pickleAppend(dst, secondArg);

    if constexpr (sizeof...(args) > 0) {
        pickleAppend(dst, args...);
    }
}

template<typename... Args>
inline void clearAndPickleAppend(std::string &dst, Args&&... args) noexcept
{
    dst.clear();
    pickleAppend(dst, args...);
}

template<typename... Args>
[[nodiscard]] inline std::string pickle(Args&&... args) noexcept
{
    auto dst = std::string{};
    pickleAppend(dst, args...);
    return dst;
}


};
