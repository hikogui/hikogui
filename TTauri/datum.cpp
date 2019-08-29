// Copyright 2019 Pokitec
// All rights reserved.

#include "datum.hpp"
#include "utils.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>

namespace TTauri {

datum::datum(datum const &other) noexcept {
    switch (other.physical_type()) {
    case datum_physical_type::StringPointer: {
        auto *p = new std::string(*other.get_pointer<std::string>());
        u64 = datum_string_pointer_mask | reinterpret_cast<uint64_t>(p);
        } break;

    case datum_physical_type::URLPointer: {
        auto *p = new URL(*other.get_pointer<URL>());
        u64 = datum_url_pointer_mask | reinterpret_cast<uint64_t>(p);
        } break;

    case datum_physical_type::IntegerPointer: {
        auto *p = new int64_t(*other.get_pointer<int64_t>());
        u64 = datum_integer_pointer_mask | reinterpret_cast<uint64_t>(p);
        } break;

    case datum_physical_type::VectorPointer: {
        auto *p = new datum::vector(*other.get_pointer<datum::vector>());
        u64 = datum_vector_pointer_mask | reinterpret_cast<uint64_t>(p);
        } break;

    case datum_physical_type::MapPointer: {
        auto *p = new datum::map(*other.get_pointer<datum::map>());
        u64 = datum_map_pointer_mask | reinterpret_cast<uint64_t>(p);
        } break;

    case datum_physical_type::Float:
        f64 = other.f64;
        break;

    default:
        u64 = other.u64;
    }
}

datum::datum(datum &&other) noexcept {
    if (other.is_float()) {
        f64 = other.f64;
    } else {
        u64 = other.u64;
    }
    other.u64 = datum_undefined_mask;
}

datum &datum::operator=(datum const &other) noexcept {
    reset();
    switch (other.physical_type()) {
    case datum_physical_type::StringPointer: {
        auto *p = new std::string(*other.get_pointer<std::string>());
        u64 = datum_string_pointer_mask | reinterpret_cast<uint64_t>(p);
        } break;

    case datum_physical_type::URLPointer: {
        auto *p = new URL(*other.get_pointer<URL>());
        u64 = datum_url_pointer_mask | reinterpret_cast<uint64_t>(p);
        } break;

    case datum_physical_type::IntegerPointer: {
        auto *p = new int64_t(*other.get_pointer<int64_t>());
        u64 = datum_integer_pointer_mask | reinterpret_cast<uint64_t>(p);
        } break;

    case datum_physical_type::VectorPointer: {
        auto *p = new datum::vector(*other.get_pointer<datum::vector>());
        u64 = datum_vector_pointer_mask | reinterpret_cast<uint64_t>(p);
        } break;

    case datum_physical_type::MapPointer: {
        auto *p = new datum::map(*other.get_pointer<datum::map>());
        u64 = datum_map_pointer_mask | reinterpret_cast<uint64_t>(p);
        } break;

    case datum_physical_type::Float:
        f64 = other.f64;
        break;

    default:
        u64 = other.u64;
    }
    return *this;
}

datum &datum::operator=(datum &&other) noexcept {
    reset();
    if (other.is_float()) {
        f64 = other.f64;
    } else {
        u64 = other.u64;
    }
    other.u64 = datum_undefined_mask;
    return *this;
}

datum::datum(int64_t value) noexcept : u64(datum_integer_mask | (value & 0x0000ffff'ffffffff)) {
    if (value < datum_min_int || value > datum_max_int) {
        // Overflow.
        auto p = new int64_t(value);
        u64 = datum_integer_pointer_mask | reinterpret_cast<uint64_t>(p);
    }
}

datum::datum(std::string_view value) noexcept : u64(datum_make_string(value)) {
    if (value.size() > 6) {
        // Overflow.
        auto p = new std::string(value);
        u64 = datum_string_pointer_mask | reinterpret_cast<uint64_t>(p);
    }
}

datum::datum(URL const &value) noexcept {
    auto p = new URL(value);
    u64 = datum_url_pointer_mask | reinterpret_cast<uint64_t>(p);
}

datum::datum(datum::vector const &value) noexcept {
    auto p = new datum::vector(value);
    u64 = datum_vector_pointer_mask | reinterpret_cast<uint64_t>(p);
}

datum::datum(datum::map const &value) noexcept {
    auto p = new datum::map(value);
    u64 = datum_map_pointer_mask | reinterpret_cast<uint64_t>(p);
}



datum::operator double() const {
    switch (physical_type()) {
    case datum_physical_type::Integer: return static_cast<double>(get_signed_integer());
    case datum_physical_type::IntegerPointer: return static_cast<double>(*get_pointer<int64_t>());
    case datum_physical_type::Float: return f64;
    default: TTAURI_THROW(invalid_operation_error("Value {} of type {} can not be converted to a double", this->repr(), this->type_name()));
    }
}

datum::operator float() const {
    return static_cast<float>(static_cast<double>(*this));
}

datum::operator int64_t() const {
    switch (physical_type()) {
    case datum_physical_type::Integer: return get_signed_integer();
    case datum_physical_type::IntegerPointer: return *get_pointer<int64_t>();
    case datum_physical_type::Float: return static_cast<int64_t>(f64);
    case datum_physical_type::Boolean: return get_unsigned_integer();
    default: TTAURI_THROW(invalid_operation_error("Value {} of type {} can not be converted to a int64_t", this->repr(), this->type_name()));
    }
}

datum::operator int32_t() const {
    let v = static_cast<int64_t>(*this);
    if (v < std::numeric_limits<int32_t>::min() || v > std::numeric_limits<int32_t>::max()) {
        TTAURI_THROW(invalid_operation_error("Value {} of type {} can not be converted to a int32_t", this->repr(), this->type_name()));
    }
    return static_cast<int32_t>(v);
}

datum::operator int16_t() const {
    let v = static_cast<int64_t>(*this);
    if (v < std::numeric_limits<int16_t>::min() || v > std::numeric_limits<int16_t>::max()) {
        TTAURI_THROW(invalid_operation_error("Value {} of type {} can not be converted to a int16_t", this->repr(), this->type_name()));
    }
    return static_cast<int16_t>(v);
}

datum::operator int8_t() const {
    let v = static_cast<int64_t>(*this);
    if (v < std::numeric_limits<int8_t>::min() || v > std::numeric_limits<int8_t>::max()) {
        TTAURI_THROW(invalid_operation_error("Value {} of type {} can not be converted to a int8_t", this->repr(), this->type_name()));
    }
    return static_cast<int8_t>(v);
}

datum::operator uint64_t() const {
    let v = static_cast<int64_t>(*this);
    return static_cast<uint64_t>(v);
}

datum::operator uint32_t() const {
    let v = static_cast<uint64_t>(*this);
    if ( v > std::numeric_limits<uint32_t>::max()) {
        TTAURI_THROW(invalid_operation_error("Value {} of type {} can not be converted to a uint32_t", this->repr(), this->type_name()));
    }
    return static_cast<uint32_t>(v);
}

datum::operator uint16_t() const {
    let v = static_cast<uint64_t>(*this);
    if ( v > std::numeric_limits<uint16_t>::max()) {
        TTAURI_THROW(invalid_operation_error("Value {} of type {} can not be converted to a uint16_t", this->repr(), this->type_name()));
    }
    return static_cast<uint16_t>(v);
}

datum::operator uint8_t() const {
    let v = static_cast<uint64_t>(*this);
    if ( v > std::numeric_limits<uint8_t>::max()) {
        TTAURI_THROW(invalid_operation_error("Value {} of type {} can not be converted to a uint8_t", this->repr(), this->type_name()));
    }
    return static_cast<uint8_t>(v);
}

datum::operator bool() const noexcept {
    switch (logical_type()) {
    case datum_logical_type::Float: {
        let v = static_cast<double>(*this);
        return v != 0.0 && v == v;
        }

    case datum_logical_type::Integer: return static_cast<int64_t>(*this) != 0;
    case datum_logical_type::Boolean: return get_unsigned_integer() > 0;
    case datum_logical_type::Null: return false;
    case datum_logical_type::Undefined: return false;
    case datum_logical_type::String: return this->size() > 0;
    case datum_logical_type::URL: return true;
    case datum_logical_type::Vector: return this->size() > 0;
    case datum_logical_type::Map: return this->size() > 0;
    default: no_default;
    }
}

datum::operator char() const {
    switch (physical_type()) {
    case datum_physical_type::String:
        if (size() == 1) {
            return u64 & 0xff;
        }

    case datum_physical_type::StringPointer:
        if (size() == 1) {
            return get_pointer<std::string>()->at(0);
        }
    }

    TTAURI_THROW(invalid_operation_error("Value {} of type {} can not be converted to a char", this->repr(), this->type_name()));
}

datum::operator std::string() const noexcept {
    switch (physical_type()) {
    case datum_physical_type::String: {
        let length = size();
        char buffer[6];
        for (int i = 0; i < length; i++) {
            buffer[i] = (u64 >> ((length - i - 1) * 8)) & 0xff;
        }
        return std::string(buffer, length);
    }
    case datum_physical_type::StringPointer: return *get_pointer<std::string>();
    case datum_physical_type::URLPointer: return get_pointer<URL>()->string();
    case datum_physical_type::Boolean: return static_cast<bool>(*this) ? "true" : "false";
    case datum_physical_type::Null: return "null";
    case datum_physical_type::Undefined: return "undefined";
    case datum_physical_type::Integer: return fmt::format("{}", static_cast<int64_t>(*this));
    case datum_physical_type::IntegerPointer: return fmt::format("{}", static_cast<int64_t>(*this));
    case datum_physical_type::VectorPointer: {
        std::string r = "[";
        auto i = 0;
        for (let &v: *get_pointer<datum::vector>()) {
            if (i++ > 0) {
                r += ", ";
            }
            r += static_cast<std::string>(v);
        }
        r += "]";
        return r;
    }

    case datum_physical_type::MapPointer: {
        std::string r = "{";
        auto i = 0;
        for (let &[k, v]: *get_pointer<datum::map>()) {
            if (i++ > 0) {
                r += ", ";
            }
            r += static_cast<std::string>(k);
            r += ": ";
            r += static_cast<std::string>(v);
        }
        r += "}";
        return r;
    }

    default: no_default;
    }
}

datum::operator URL() const {
    switch (logical_type()) {
    case datum_logical_type::String: return URL{static_cast<std::string>(*this)};
    case datum_logical_type::URL: return *get_pointer<URL>();
    default: TTAURI_THROW(invalid_operation_error("Value {} of type {} can not be converted to a URL", this->repr(), this->type_name()));
    }
}

datum::operator datum::vector() const {
    if (is_vector()) {
        return *get_pointer<datum::vector>();
    }

    TTAURI_THROW(invalid_operation_error("Value {} of type {} can not be converted to a char", this->repr(), this->type_name()));
}

datum::operator datum::map() const {
    if (is_map()) {
        return *get_pointer<datum::map>();
    }

    TTAURI_THROW(invalid_operation_error("Value {} of type {} can not be converted to a char", this->repr(), this->type_name()));
}

void datum::reset() noexcept {
    if (holds_pointer()) {
        delete get_pointer<void>();
    }
    u64 = datum_undefined_mask;
}

uint8_t datum::type_id() const noexcept {
    constexpr uint16_t exponent_mask = 0b0111'1111'1111'0000;
    constexpr uint16_t type_mask = 0b0000'0000'0000'1111;

    // We use bit_cast<> on this to get access to the
    // actual bytes for determining the stored type.
    // This gets arround C++ undefined behavour of type-punning
    // through a union.
    std::byte bytes[sizeof(datum)];
    std::memcpy(bytes, this, sizeof(bytes));

    let ms_word =
        (static_cast<uint16_t>(bytes[7]) << 8) |
        static_cast<uint16_t>(bytes[6]);

    if ((ms_word & exponent_mask) != exponent_mask) {
        return 0;
    }

    // Get the type, lower 4 bits + the sign bit.
    return static_cast<uint8_t>((ms_word & type_mask) | (ms_word >> 11));
}

char const *datum::type_name() const noexcept {
    switch (logical_type()) {
    case datum_logical_type::Float: return "Float";
    case datum_logical_type::Integer: return "Integer";
    case datum_logical_type::Boolean: return "Boolean";
    case datum_logical_type::Null: return "Null";
    case datum_logical_type::Undefined: return "Undefined";
    case datum_logical_type::String: return "String";
    case datum_logical_type::URL: return "URL";
    case datum_logical_type::Vector: return "Vector";
    case datum_logical_type::Map: return "Map";
    default: no_default;
    }
}

std::string datum::repr() const noexcept {
    switch (logical_type()) {
    case datum_logical_type::Float: return static_cast<std::string>(*this);
    case datum_logical_type::Integer: return static_cast<std::string>(*this);
    case datum_logical_type::Boolean: return static_cast<std::string>(*this);
    case datum_logical_type::Null: return static_cast<std::string>(*this);
    case datum_logical_type::Undefined: return static_cast<std::string>(*this);
    case datum_logical_type::String: return fmt::format("\"{}\"", static_cast<std::string>(*this));
    case datum_logical_type::URL: return fmt::format("<URL {}>", static_cast<std::string>(*this));
    case datum_logical_type::Vector: return static_cast<std::string>(*this);
    case datum_logical_type::Map: return static_cast<std::string>(*this);
    default: no_default;
    }
}

size_t datum::size() const {
    switch (physical_type()) {
    case datum_physical_type::String: return to_int(((u64 & 0xffff'0000'0000'0000ULL) - datum_string_mask) >> 48);
    case datum_physical_type::StringPointer: return get_pointer<std::string>()->size();
    case datum_physical_type::VectorPointer: return get_pointer<datum::vector>()->size();
    case datum_physical_type::MapPointer: return get_pointer<datum::map>()->size();
    default: TTAURI_THROW(invalid_operation_error("Can't get size of value {} of type {}.", this->repr(), this->type_name()));
    }
}

size_t datum::hash() const noexcept {
    switch (physical_type()) {
    case datum_physical_type::StringPointer: return std::hash<std::string>{}(*get_pointer<std::string>());
    case datum_physical_type::URLPointer: return std::hash<URL>{}(*get_pointer<URL>());
    case datum_physical_type::VectorPointer: return std::hash<datum::vector>{}(*get_pointer<datum::vector>());
    case datum_physical_type::MapPointer: return std::hash<datum::map>{}(*get_pointer<datum::map>());
    case datum_physical_type::Float: return std::hash<double>{}(f64);
    default: return std::hash<uint64_t>{}(u64);
    }
}

bool operator<(datum::map const &lhs, datum::map const &rhs) noexcept
{
    auto lhs_keys = transform<datum::vector>(lhs, [](auto x) { return x.first; });
    auto rhs_keys = transform<datum::vector>(lhs, [](auto x) { return x.first; });

    if (lhs_keys == rhs_keys) {
        for (let &k: lhs_keys) {
            if (lhs.at(k) == rhs.at(k)) {
                continue;
            } else if (lhs.at(k) < rhs.at(k)) {
                return true;
            } else {
                return false;
            }
        }
    } else {
        return lhs_keys < rhs_keys;
    }
    return false;
}

bool operator==(datum const &lhs, datum const &rhs) noexcept
{
    switch (lhs.logical_type()) {
    case datum_logical_type::Boolean:
        return rhs.is_boolean() && static_cast<bool>(lhs) == static_cast<bool>(rhs);
    case datum_logical_type::Float:
        return rhs.is_numeric() && static_cast<double>(lhs) == static_cast<double>(rhs);
    case datum_logical_type::Integer:
        switch (rhs.logical_type()) {
        case datum_logical_type::Float: return static_cast<double>(lhs) == static_cast<double>(rhs);
        case datum_logical_type::Integer: return static_cast<int64_t>(lhs) == static_cast<int64_t>(rhs);
        default: false;
        }
    case datum_logical_type::Null:
        return rhs.is_null();
    case datum_logical_type::Undefined:
        return rhs.is_undefined();
    case datum_logical_type::String:
        switch (rhs.logical_type()) {
        case datum_logical_type::String: return static_cast<std::string>(lhs) == static_cast<std::string>(rhs);
        case datum_logical_type::URL: return static_cast<URL>(lhs) == static_cast<URL>(rhs);
        default: return false;
        }
    case datum_logical_type::URL:
        return (rhs.is_url() || rhs.is_string()) && static_cast<URL>(lhs) == static_cast<URL>(rhs);
    case datum_logical_type::Vector:
        return rhs.is_vector() && *lhs.get_pointer<datum::vector>() == *rhs.get_pointer<datum::vector>();
    case datum_logical_type::Map:
        return rhs.is_vector() && *lhs.get_pointer<datum::map>() == *rhs.get_pointer<datum::map>();
    default:
        no_default;
    }
}

bool operator<(datum const &lhs, datum const &rhs) noexcept
{
    switch (lhs.logical_type()) {
    case datum_logical_type::Boolean:
        if (rhs.is_boolean()) {
            return static_cast<bool>(lhs) < static_cast<bool>(rhs);
        } else {
            return static_cast<int>(lhs.logical_type()) < static_cast<int>(rhs.logical_type());
        }
    case datum_logical_type::Float:
        if (rhs.is_numeric()) {
            return static_cast<double>(lhs) < static_cast<double>(rhs);
        } else {
            return static_cast<int>(lhs.logical_type()) < static_cast<int>(rhs.logical_type());
        }

    case datum_logical_type::Integer:
        switch (rhs.logical_type()) {
        case datum_logical_type::Float: return static_cast<double>(lhs) < static_cast<double>(rhs);
        case datum_logical_type::Integer: return static_cast<int64_t>(lhs) < static_cast<int64_t>(rhs);
        default: return static_cast<int>(lhs.logical_type()) < static_cast<int>(rhs.logical_type());
        }
    case datum_logical_type::Null:
        return static_cast<int>(lhs.logical_type()) < static_cast<int>(rhs.logical_type());
    case datum_logical_type::Undefined:
        return static_cast<int>(lhs.logical_type()) < static_cast<int>(rhs.logical_type());
    case datum_logical_type::String:
        switch (rhs.logical_type()) {
        case datum_logical_type::String: return static_cast<std::string>(lhs) < static_cast<std::string>(rhs);
        case datum_logical_type::URL: return static_cast<URL>(lhs) < static_cast<URL>(rhs);
        default: return static_cast<int>(lhs.logical_type()) < static_cast<int>(rhs.logical_type());
        }
    case datum_logical_type::URL:
        if (rhs.is_url() || rhs.is_string()) {
            return static_cast<URL>(lhs) < static_cast<URL>(rhs);
        } else {
            return static_cast<int>(lhs.logical_type()) < static_cast<int>(rhs.logical_type());
        }
    case datum_logical_type::Vector:
        if (rhs.is_vector()) {
            return *lhs.get_pointer<datum::vector>() < *rhs.get_pointer<datum::vector>();
        } else {
            return static_cast<int>(lhs.logical_type()) < static_cast<int>(rhs.logical_type());
        }
    case datum_logical_type::Map:
        if (rhs.is_vector()) {
            return *lhs.get_pointer<datum::map>() < *rhs.get_pointer<datum::map>();
        } else {
            return static_cast<int>(lhs.logical_type()) < static_cast<int>(rhs.logical_type());
        }
    default:
        no_default;
    }
}

datum operator+(datum const &lhs, datum const &rhs)
{
    if (lhs.is_integer() && rhs.is_integer()) {
        let lhs_ = static_cast<int64_t>(lhs);
        let rhs_ = static_cast<int64_t>(rhs);
        return datum{lhs_ + rhs_};

    } else if (lhs.is_numeric() && rhs.is_numeric()) {
        let lhs_ = static_cast<double>(lhs);
        let rhs_ = static_cast<double>(rhs);
        return datum{lhs_ + rhs_};

    } else if (lhs.is_string() && rhs.is_string()) {
        let lhs_ = static_cast<std::string>(lhs);
        let rhs_ = static_cast<std::string>(rhs);
        return datum{lhs_ + rhs_};

    } else {
        TTAURI_THROW(invalid_operation_error("Can't add '+' value {} of type {} to value {} of type {}",
            lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
        ));
    }
}

}