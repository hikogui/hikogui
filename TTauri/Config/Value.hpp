// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/utils.hpp"
#include "TTauri/Color.hpp"
#include <boost/format.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <any>
#include <map>
#include <vector>
#include <cstdint>
#include <cmath>
#include <filesystem>

namespace TTauri::Config {

struct Value;

struct Undefined {};

using Object = std::map<std::string, Value>;
using Array = std::vector<Value>;

enum class CompareResult {
    LOWER,
    SAME,
    HIGHER
};

inline CompareResult compare(std::string l, std::string r)
{
    auto result = l.compare(r);
    if (result < 0) {
        return CompareResult::LOWER;
    } else if (result > 0) {
        return CompareResult::HIGHER;
    } else {
        return CompareResult::SAME;
    }
}

inline CompareResult compare(int64_t l, int64_t r)
{
    if (l < r) {
        return CompareResult::LOWER;
    } else if (l > r) {
        return CompareResult::HIGHER;
    } else {
        return CompareResult::SAME;
    }
}

inline CompareResult compare(bool l, bool r) {
    if (l == r) {
        return CompareResult::SAME;
    } else if (l) {
        return CompareResult::HIGHER;
    } else {
        return CompareResult::LOWER;
    }
}

inline CompareResult compare(double l, double r)
{
    int64_t l_int; memcpy(&l_int, &l, sizeof(l_int));
    int64_t r_int; memcpy(&r_int, &r, sizeof(r_int));

    auto l_positive = (l_int & 0x8000'0000'0000'0000ULL) == 0;
    auto r_positive = (r_int & 0x8000'0000'0000'0000ULL) == 0;
    l_int &= 0x7fff'ffff'ffff'ffffULL;
    r_int &= 0x7fff'ffff'ffff'ffffULL;

    if (l_positive == r_positive) {
        int64_t diff = l_int - r_int;
        if (diff > 5) {
            return CompareResult::HIGHER;
        } else if (diff < -5) {
            return CompareResult::LOWER;
        } else {
            return CompareResult::SAME;
        }
    } else {
        int64_t diff = r_int + l_int;

        if (diff <= 10) {
            return CompareResult::SAME;
        } else if (l_positive) {
            return CompareResult::HIGHER;
        } else {
            return CompareResult::LOWER;
        }
    }
}


/*! A generic value type which will handle intra type operations.
 */
struct Value {
    std::any intrinsic;

    /*! Exception raised when an operation is not allowed by the contained type.
     */
    struct InvalidOperationError : virtual boost::exception, virtual std::exception {};

    Value() : intrinsic({}) {}

    Value(std::any value) : intrinsic(value) {}

    Value(const Value &) = default;

    Value(Value &&) = default;

    ~Value() = default;

    Value &operator=(const Value &) = default;

    Value &operator=(Value &&) = default;

    Value &operator=(std::any value) {
        intrinsic = std::move(value);
        return *this;
    }

    bool has_value() const {
        return intrinsic.has_value();
    }

    std::type_info const &type() const noexcept {
        return intrinsic.type();
    }

    template<typename T>
    bool is_type() const {
        return type() == typeid(T);
    }

    template<typename T>
    T value() const {
        return std::any_cast<T>(intrinsic);
    }

    template<typename T>
    T &value() {
        return std::any_cast<T &>(intrinsic);
    }

    /*! Return a string representation of the value.
     * \return a string representing the value.
     */
    std::string str() const {
        if (is_type<bool>()) {
            return value<bool>() ? "true" : "false";
        } else if (!has_value()) {
            return "null";
        } else if (is_type<int64_t>()) {
            return (boost::format("%i") % value<int64_t>()).str();
        } else if (is_type<double>()) {
            return (boost::format("%g") % value<double>()).str();
        } else if (is_type<Color_sRGB>()) {
            return value<Color_sRGB>().str();
        } else if (is_type<std::string>()) {
            return "\"" + value<std::string>() + "\"";
        } else if (is_type<std::filesystem::path>()) {
            return "\"" + value<std::filesystem::path>().string() + "\"";
        } else if (is_type<Array>()) {
            std::string s = "[";
            auto first = true;
            for (auto const &x: value<Array>()) {
                if (!first) {
                    s += ",";
                }
                s += x.str();
                first = false;
            }
            return s + "]";
        } else if (is_type<Object>()) {
            std::string s = "{";
            auto first = true;
            for (auto const &[k, v]: value<Object>()) {
                if (!v.is_type<Undefined>()) {
                    if (!first) {
                        s += ",";
                    }
                    s += k + ":" + v.str();
                    first = false;
                }
            }
            return s + "}";
        }
        BOOST_THROW_EXCEPTION(NotImplementedError());
    }

    /*! Return the internal any-value.
     * The returned any-value is potentially simplified for Arrays and Objects.
     * \return the std:any value,
     */
    std::any any() const {
        if (is_type<Array>()) {
            std::vector<std::any> r;
            for (auto const &x: value<Array>()) {
                r.push_back(x.any());
            }
            return r;

        } else if (is_type<Object>()) {
            std::map<std::string, std::any> r;
            for (auto const &[k, v]: value<Object>()) {
                if (!v.is_type<Undefined>()) {
                    r[k] = v.any();
                }
            }
            return r;

        } else {
            return intrinsic;
        }
    }

    Value boolean() const {
        if (!has_value()) {
            return false;
        } else if (is_type<bool>()) {
            return value<bool>();
        } else if (is_type<int64_t>()) {
            return value<int64_t>() != 0;
        } else if (is_type<double>()) {
            return value<double>() != 0.0;
        } else if (is_type<Color_sRGB>()) {
            return value<Color_sRGB>().a() != 0.0; // Not transparent
        } else if (is_type<std::string>()) {
            return value<std::string>().size() > 0;
        } else if (is_type<std::filesystem::path>()) {
            return true;
        } else if (is_type<Array>()) {
            return value<Array>().size() > 0;
        } else if (is_type<Object>()) {
            return value<Object>().size() > 0;
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    CompareResult cmp(Value const &other) const {
        if (is_type<std::string>() && other.is_type<std::string>()) {
            return compare(value<std::string>(), other.value<std::string>());

        } else if (is_type<Array>() && other.is_type<Array>()) {
            auto l = value<Array>();
            auto r = other.value<Array>();

            auto l_iterator = l.begin();
            auto r_iterator = r.begin();
            while (l_iterator != l.end() && r_iterator != r.end()) {
                auto const l_value = *l_iterator;
                auto const r_value = *r_iterator;

                switch (l_value.cmp(r_value)) {
                case CompareResult::LOWER: return CompareResult::LOWER;
                case CompareResult::HIGHER: return CompareResult::HIGHER;
                case CompareResult::SAME:;
                }

                l_iterator++;
                r_iterator++;
            }

            if (l_iterator != l.end()) {
                return CompareResult::HIGHER;
            } else if (r_iterator != r.end()) {
                return CompareResult::LOWER;
            } else {
                return CompareResult::SAME;
            }

        } else if (is_type<Object>() && other.is_type<Object>()) {
            auto l = value<Object>();
            auto r = other.value<Object>();

            auto l_iterator = l.begin();
            auto r_iterator = r.begin();
            while (l_iterator != l.end() && r_iterator != r.end()) {
                auto const [l_key, l_value] = *l_iterator;
                auto const [r_key, r_value] = *r_iterator;

                switch (compare(l_key, r_key)) {
                case CompareResult::LOWER: return CompareResult::LOWER;
                case CompareResult::HIGHER: return CompareResult::HIGHER;
                case CompareResult::SAME:;
                }

                switch (l_value.cmp(r_value)) {
                case CompareResult::LOWER: return CompareResult::LOWER;
                case CompareResult::HIGHER: return CompareResult::HIGHER;
                case CompareResult::SAME:;
                }

                l_iterator++;
                r_iterator++;
            }

            if (l_iterator != l.end()) {
                return CompareResult::HIGHER;
            } else if (r_iterator != r.end()) {
                return CompareResult::LOWER;
            } else {
                return CompareResult::SAME;
            }

        } else if (is_type<double>() || other.is_type<double>()) {
            return compare(value<double>(), other.value<double>());
        } else if (is_type<int64_t>() || other.is_type<int64_t>()) {
            return compare(value<int64_t>(), other.value<int64_t>());
        } else if (is_type<bool>() || other.is_type<bool>()) {
            return compare(value<bool>(), other.value<bool>());
        }

        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    Value operator-() const {
        if (is_type<int64_t>()) {
            return -value<int64_t>();
        } else if (is_type<double>()) {
            return -value<double>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    Value operator~() const {
        if (is_type<int64_t>()) {
            return ~value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    Value operator!() const {
        if (is_type<bool>()) {
            return !value<bool>();
        } else {
            return !boolean();
        }
    }

    Value operator*(Value const &other) const {
        if (is_type<double>() || other.is_type<double>()) {
            return value<double>() * other.value<double>();
        } else if (is_type<int64_t>() || other.is_type<int64_t>()) {
            return value<int64_t>() * other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    Value operator/(Value const &other) const {
        if (is_type<double>() || other.is_type<double>()) {
            return value<double>() / other.value<double>();
        } else if (is_type<int64_t>() || other.is_type<int64_t>()) {
            return value<int64_t>() / other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    Value operator%(Value const &other) const {
        if (is_type<double>() || other.is_type<double>()) {
            return fmod(value<double>(), other.value<double>());
        } else if (is_type<int64_t>() || other.is_type<int64_t>()) {
            return value<int64_t>() % other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    Value operator+(Value const &other) const {
        if (is_type<std::filesystem::path>() && other.is_type<std::string>()) {
            return value<std::filesystem::path>() / other.value<std::string>();
        } else if (is_type<std::filesystem::path>() && other.is_type<std::filesystem::path>()) {
            return value<std::filesystem::path>() / other.value<std::filesystem::path>();
        } else if (is_type<std::string>() && other.is_type<std::string>()) {
            return value<std::string>() + other.value<std::string>();
        } else if (is_type<Array>() && other.is_type<Array>()) {
            Array r;
            for (auto const x: value<Array>()) { r.push_back(x); }
            for (auto const x: other.value<Array>()) { r.push_back(x); }
            return r;
        } else if (is_type<Object>() && other.is_type<Object>()) {
            Object r;
            for (auto const [k, v]: value<Object>()) { r[k] = v; }
            for (auto const [k, v]: other.value<Object>()) { r[k] = v; }
            return r;
        } else if (is_type<double>() || other.is_type<double>()) {
            return value<double>() + other.value<double>();
        } else if (is_type<int64_t>() || other.is_type<int64_t>()) {
            return value<int64_t>() + other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    Value operator-(Value const &other) const {
        if (is_type<double>() || other.is_type<double>()) {
            return value<double>() - other.value<double>();
        } else if (is_type<int64_t>() || other.is_type<int64_t>()) {
            return value<int64_t>() - other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    Value operator<<(Value const &other) const {
        if (is_type<int64_t>() && other.is_type<int64_t>()) {
            return value<int64_t>() << other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    Value operator>>(Value const &other) const {
        if (is_type<int64_t>() && other.is_type<int64_t>()) {
            return value<int64_t>() >> other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    Value operator<(Value const &other) const {
        return cmp(other) == CompareResult::LOWER;
    }

    Value operator>(Value const &other) const {
        return cmp(other) == CompareResult::HIGHER;
    }

    Value operator<=(Value const &other) const {
        return cmp(other) != CompareResult::HIGHER;
    }

    Value operator>=(Value const &other) const {
        return cmp(other) != CompareResult::LOWER;
    }

    Value operator==(Value const &other) const {
        return cmp(other) == CompareResult::SAME;
    }

    Value operator!=(Value const &other) const {
        return cmp(other) != CompareResult::SAME;
    }

    Value operator&(Value const &other) const {
        if (is_type<int64_t>() && other.is_type<int64_t>()) {
            return value<int64_t>() & other.value<int64_t>();
        } else if (is_type<bool>() && other.is_type<bool>()) {
            return value<bool>() & other.value<bool>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    Value operator^(Value const &other) const {
        if (is_type<int64_t>() && other.is_type<int64_t>()) {
            return value<int64_t>() ^ other.value<int64_t>();
        } else if (is_type<bool>() && other.is_type<bool>()) {
            return value<bool>() ^ other.value<bool>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    Value operator|(Value const &other) const {
        if (is_type<int64_t>() && other.is_type<int64_t>()) {
            return value<int64_t>() & other.value<int64_t>();
        } else if (is_type<bool>() && other.is_type<bool>()) {
            return value<bool>() & other.value<bool>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    Value operator&&(Value const &other) const {
        if (is_type<bool>() && other.is_type<bool>()) {
            return value<bool>() & other.value<bool>();
        } else {
            return boolean() && other.boolean();
        }
    }

    Value operator||(Value const &other) const {
        if (is_type<bool>() && other.is_type<bool>()) {
            return value<bool>() | other.value<bool>();
        } else {
            return boolean() || other.boolean();
        }
    }

    Value operator_xor(Value const &other) const {
        if (is_type<bool>() && other.is_type<bool>()) {
            return value<bool>() ^ other.value<bool>();
        } else {
            return boolean() || other.boolean();
        }
    }

    Value &operator[](Value const &other) {
        if (is_type<Undefined>()) {
            if (other.is_type<int64_t>()) {
                intrinsic = Array{};
            } else if (other.is_type<std::string>()) {
                intrinsic = Object{};
            }
        }

        if (is_type<Array>() && other.is_type<int64_t>()) {
            auto &_array = value<Array>();
            size_t index = boost::numeric_cast<size_t>(other.value<int64_t>());
            while (index >= _array.size()) {
                _array.emplace_back(Undefined{});
            }

            return _array.at(index);

        } else if (is_type<Object>() && other.is_type<std::string>()) {
            auto &obj = value<Object>();
            auto const &key = other.value<std::string>();
            obj.try_emplace(key, Undefined{});
            return obj[key];
        }

        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

    virtual Value &operator[](std::string const &other) {
        if (is_type<Undefined>()) {
            // When accessing a name on an undefined it means we need replace it with an empty object.
            intrinsic = Object{};
        }

        if (is_type<Object>()) {
            auto &obj = value<Object>();
            obj.try_emplace(other, Undefined{});
            return obj[other];
        }

        BOOST_THROW_EXCEPTION(InvalidOperationError());
    }

};

}
