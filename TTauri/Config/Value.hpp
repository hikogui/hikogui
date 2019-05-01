// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/utils.hpp"
#include "TTauri/Color.hpp"
#include "exceptions.hpp"
#include <boost/format.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <any>
#include <map>
#include <vector>
#include <cstdint>
#include <cmath>
#include <filesystem>
#include <typeinfo>
#include <string>

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

    Value() : intrinsic(Undefined{}) {}

    Value(std::any value) : intrinsic(value) {
        if (!is_valid_type()) {
            BOOST_THROW_EXCEPTION(NotImplementedError()
                << errinfo_message((boost::format("Assigning a value of type %s is not implemented") % type_name()).str())        
            );
        }
    }

    Value(const Value &) = default;

    Value(Value &&) = default;

    ~Value() = default;

    Value &operator=(const Value &) = default;

    Value &operator=(Value &&) = default;

    Value &operator=(std::any value) {
        intrinsic = std::move(value);

        if (!is_valid_type()) {
            BOOST_THROW_EXCEPTION(NotImplementedError()
                << errinfo_message((boost::format("Assigning a value of type %s is not implemented") % type_name()).str())        
            );
        }

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

    bool is_valid_type() const {
        return is_type<void>() || is_type<bool>() || is_type<int64_t>() || is_type<double>() || is_type<std::string>() || is_type<std::filesystem::path>() ||
            is_type<Color_XYZ>() || is_type<Object>() || is_type<Array>() || is_type<Undefined>();
    }

    template<typename T>
    bool is_promotable_to() const {
        return (
            (typeid(T) == typeid(double) && is_type<uint64_t>()) ||
            (typeid(T) == typeid(std::filesystem::path) && is_type<std::string>()) ||
            is_type<T>()
        );
    }

    template<typename T>
    T value() const {
        return std::any_cast<T>(intrinsic);
    }

    template<typename T>
    T &value() {
        return std::any_cast<T &>(intrinsic);
    }

    template<>
    double value() const {
        if (is_type<int64_t>()) {
            return static_cast<double>(std::any_cast<int64_t>(intrinsic));
        } else {
            return std::any_cast<double>(intrinsic);
        }
    }

    template<>
    std::filesystem::path value() const {
        if (is_type<std::string>()) {
            return std::filesystem::path{std::any_cast<std::string>(intrinsic)};
        } else {
            return std::any_cast<std::filesystem::path>(intrinsic);
        }
    }

    template<>
    std::vector<std::any> value() const {
        return std::any_cast<std::vector<std::any>>(any());
    }

    template<>
    std::map<std::string, std::any> value() const {
        return std::any_cast<std::map<std::string, std::any>>(any());
    }

    std::string type_name() const {
        return type().name();
    }

    Value const &get(std::vector<std::string> const &key) {
        if (is_type<Object>()) {
            size_t const index = std::stoll(key.at(0));
            auto next = (*this)[index];
            return next.get(std::vector<std::string>{key.begin() + 1, key.end()});

        } else if (is_type<Array>()) {
            auto const index = key.at(0);
            auto next = (*this)[index];
            return next.get(std::vector<std::string>{key.begin() + 1, key.end()});

        } else if (key.size() > 0) {
            BOOST_THROW_EXCEPTION(InvalidOperationError()
                << errinfo_message((boost::format("type %s does not support get() with '%s'")
                    % type_name() % key.at(0)).str())        
            );
        } else {
            return *this;
        }
    }

    Value const &get(std::string const &key) {
        return get(split(key, '.'));
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
            auto s = (boost::format("%g") % value<double>()).str();
            if (s.find('.') == s.npos) {
                return s + ".";
            } else {
                return s;
            }
        } else if (is_type<Color_XYZ>()) {
            return color_cast<Color_sRGB>(value<Color_XYZ>()).str();
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
        BOOST_THROW_EXCEPTION(NotImplementedError()
            << errinfo_message((boost::format("type %s does not implement .str()") % type_name()).str())        
        );
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
        } else if (is_type<Color_XYZ>()) {
            return value<Color_XYZ>().a() != 0.0; // Not transparent
        } else if (is_type<std::string>()) {
            return value<std::string>().size() > 0;
        } else if (is_type<std::filesystem::path>()) {
            return true;
        } else if (is_type<Array>()) {
            return value<Array>().size() > 0;
        } else if (is_type<Object>()) {
            return value<Object>().size() > 0;
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("type %s does not implement .boolean()") % type_name()).str())
        );
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

        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("Cannot compare values of types %s and %s") % type_name() % other.type_name()).str())        
        );
    }

    Value operator-() const {
        if (is_type<int64_t>()) {
            return -value<int64_t>();
        } else if (is_type<double>()) {
            return -value<double>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("Cannot make value of type %s negative") % type_name()).str())        
        );
    }

    Value operator~() const {
        if (is_type<int64_t>()) {
            return ~value<int64_t>();
        } else if (is_type<bool>()) {
            return !value<bool>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("Cannot invert value of type %s") % type_name()).str())        
        );
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
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("Cannot multiple value of type %s with value of type %s") % type_name() % other.type_name()).str())        
        );
    }

    Value operator/(Value const &other) const {
        if (is_type<double>() || other.is_type<double>()) {
            return value<double>() / other.value<double>();
        } else if (is_type<int64_t>() || other.is_type<int64_t>()) {
            return value<int64_t>() / other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("Cannot divide value of type %s with value of type %s") % type_name() % other.type_name()).str())        
        );
    }

    Value operator%(Value const &other) const {
        if (is_type<double>() || other.is_type<double>()) {
            return fmod(value<double>(), other.value<double>());
        } else if (is_type<int64_t>() || other.is_type<int64_t>()) {
            return value<int64_t>() % other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("Cannot take modulo of value of type %s with value of type %s") % type_name() % other.type_name()).str())        
        );
    }

    Value operator+(Value const &other) const {
        if (is_type<std::filesystem::path>() || other.is_type<std::filesystem::path>()) {
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
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("Cannot add value of type %s to a value of type %s") % other.type_name() % type_name()).str())        
        );
    }

    Value operator-(Value const &other) const {
        if (is_type<double>() || other.is_type<double>()) {
            return value<double>() - other.value<double>();
        } else if (is_type<int64_t>() || other.is_type<int64_t>()) {
            return value<int64_t>() - other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("Cannot subtract value of type %s from a value of type %s") % other.type_name() % type_name()).str())        
        );
    }

    Value operator<<(Value const &other) const {
        if (is_type<int64_t>() && other.is_type<int64_t>()) {
            return value<int64_t>() << other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("Cannot left shift a of value of type %s with a value of type %s") % type_name() % other.type_name()).str())        
        );
    }

    Value operator>>(Value const &other) const {
        if (is_type<int64_t>() && other.is_type<int64_t>()) {
            return value<int64_t>() >> other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("Cannot right shift a of value of type %s with a value of type %s") % type_name() % other.type_name()).str())        
        );
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
            return value<bool>() && other.value<bool>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("Cannot binary-and a of value of type %s to a value of type %s") % other.type_name() % type_name() ).str())        
        );
    }

    Value operator^(Value const &other) const {
        if (is_type<int64_t>() && other.is_type<int64_t>()) {
            return value<int64_t>() ^ other.value<int64_t>();
        } else if (is_type<bool>() && other.is_type<bool>()) {
            return static_cast<bool>(value<bool>() ^ other.value<bool>());
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("Cannot binary-xor a of value of type %s to a value of type %s") % other.type_name() % type_name() ).str())        
        );
    }

    Value operator|(Value const &other) const {
        if (is_type<int64_t>() && other.is_type<int64_t>()) {
            return value<int64_t>() | other.value<int64_t>();
        } else if (is_type<bool>() && other.is_type<bool>()) {
            return value<bool>() || other.value<bool>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("Cannot binary-or a of value of type %s to a value of type %s") % other.type_name() % type_name() ).str())        
        );
    }

    Value operator&&(Value const &other) const {
        if (is_type<bool>() && other.is_type<bool>()) {
            return value<bool>() && other.value<bool>();
        } else {
            return boolean() && other.boolean();
        }
    }

    Value operator||(Value const &other) const {
        if (is_type<bool>() && other.is_type<bool>()) {
            return value<bool>() || other.value<bool>();
        } else {
            return boolean() || other.boolean();
        }
    }

    Value operator_xor(Value const &other) const {
        if (is_type<bool>() && other.is_type<bool>()) {
            return static_cast<bool>(value<bool>() ^ other.value<bool>());
        } else {
            return boolean().operator_xor(other.boolean());
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

        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("Cannot index of value of type %s with a value of type %s") % type_name() % other.type_name()).str())        
        );
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

        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("Cannot get member .%s of type %s") % other % type_name()).str())        
        );
    }

    virtual Value &operator[](size_t const index) {
        if (is_type<Undefined>()) {
            // When accessing a name on an undefined it means we need replace it with an empty object.
            intrinsic = Array{};
        }

        if (is_type<Array>()) {
            auto &_array = value<Array>();
            while (index >= _array.size()) {
                _array.emplace_back(Undefined{});
            }

            return _array.at(index);
        }

        BOOST_THROW_EXCEPTION(InvalidOperationError()
            << errinfo_message((boost::format("Cannot get item at index %i of type %s") % index % type_name()).str())        
        );
    }
};

}
