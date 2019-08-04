// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Color.hpp"
#include "TTauri/required.hpp"
#include "TTauri/indirect_value.hpp"
#include "exceptions.hpp"
#include <boost/format.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <variant>
#include <any>
#include <map>
#include <vector>
#include <cstdint>
#include <cmath>
#include <boost/filesystem.hpp>
#include <typeinfo>
#include <string>
#include <memory>

namespace TTauri::Config {

struct Value;

struct Undefined {};

using Object = std::map<std::string, indirect_value<Value>>;
using Array = std::vector<indirect_value<Value>>;

enum class CompareResult { LOWER, SAME, HIGHER };

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
    using all_types = std::variant<std::monostate,bool,int64_t,double,std::string,boost::filesystem::path,wsRGBA,Object,Array,Undefined>;

    all_types intrinsic;

    Value() : intrinsic() {}
    Value(bool value) : intrinsic(value) {}
    Value(int64_t value) : intrinsic(value) {}
    Value(double value) : intrinsic(value) {}
    Value(std::string value) : intrinsic(std::move(value)) {}
    Value(boost::filesystem::path value) : intrinsic(std::move(value)) {}
    Value(wsRGBA value) : intrinsic(value) {}
    Value(Object value) : intrinsic(std::move(value)) {}
    Value(Array value) : intrinsic(std::move(value)) {}
    Value(Undefined value) : intrinsic(std::move(value)) {}

    Value(const Value &) = default;
    Value(Value &&) = default;
    ~Value() = default;
    Value &operator=(const Value &) = default;
    Value &operator=(Value &&) = default;

    Value &operator=(bool value) { intrinsic = value; return *this; }
    Value &operator=(int64_t value) { intrinsic = value; return *this; }
    Value &operator=(double value) { intrinsic = value; return *this; }
    Value &operator=(std::string value) { intrinsic = std::move(value); return *this; }
    Value &operator=(boost::filesystem::path value) { intrinsic = std::move(value); return *this; }
    Value &operator=(wsRGBA value) { intrinsic = value; return *this; }
    Value &operator=(Object value) { intrinsic = std::move(value); return *this; }
    Value &operator=(Array value) { intrinsic = std::move(value); return *this; }
    Value &operator=(Undefined value) { intrinsic = std::move(value); return *this; }

    bool has_value() const {
        return std::holds_alternative<std::monostate>(intrinsic);
    }

    std::type_info const &type() const noexcept {
        if (std::holds_alternative<std::monostate>(intrinsic)) {
            return typeid(void);
        } else if (std::holds_alternative<bool>(intrinsic)) {
            return typeid(bool);
        } else if (std::holds_alternative<int64_t>(intrinsic)) {
            return typeid(int64_t);
        } else if (std::holds_alternative<double>(intrinsic)) {
            return typeid(double);
        } else if (std::holds_alternative<std::string>(intrinsic)) {
            return typeid(std::string);
        } else if (std::holds_alternative<boost::filesystem::path>(intrinsic)) {
            return typeid(boost::filesystem::path);
        } else if (std::holds_alternative<wsRGBA>(intrinsic)) {
            return typeid(wsRGBA);
        } else if (std::holds_alternative<Object>(intrinsic)) {
            return typeid(Object);
        } else if (std::holds_alternative<Array>(intrinsic)) {
            return typeid(Array);
        } else if (std::holds_alternative<Undefined>(intrinsic)) {
            return typeid(Undefined);
        } else {
            no_default;
        }
    }

    template<typename T>
    bool is_type() const {
        return std::holds_alternative<T>(intrinsic);
    }

    template<typename T>
    bool is_promotable_to() const {
        if constexpr (std::is_same_v<std::remove_const_t<T>, double>) {
            return is_type<int64_t>();
        } else if constexpr (std::is_same_v<std::remove_const_t<T>, boost::filesystem::path>) {
            return is_type<std::string>();
        } else {
            return is_type<T>();
        }
    }

    template<typename T>
    T &value() {
        return std::get<T>(intrinsic);
    }

    template<typename T>
    T value() const {
        return std::get<T>(intrinsic);
    }

    template<>
    double value() const {
        if (is_type<int64_t>()) {
            return static_cast<double>(std::get<int64_t>(intrinsic));
        } else {
            return std::get<double>(intrinsic);
        }
    }

    template<>
    size_t value() const {
        return boost::numeric_cast<size_t>(value<int64_t>());
    }

    template<>
    boost::filesystem::path value() const {
        if (is_type<std::string>()) {
            return boost::filesystem::path{std::get<std::string>(intrinsic)};
        } else {
            return std::get<boost::filesystem::path>(intrinsic);
        }
    }

    std::string type_name() const {
        return type().name();
    }

    Value &get(std::vector<std::string> const &key) {
        if (key.size() > 0 && is_type<Object>()) {
            let index = key.at(0);
            auto &next = (*this)[index];
            return next.get({key.begin() + 1, key.end()});

        } else if (key.size() > 0 && is_type<Array>()) {
            size_t const index = std::stoll(key.at(0));
            auto &next = (*this)[index];
            return next.get({key.begin() + 1, key.end()});

        } else if (key.size() > 0) {
            BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("type %s does not support get() with '%s'")
                % type_name() % key.at(0)).str())
            );
        } else {
            return *this;
        }
    }

    Value get(std::vector<std::string> const &key) const {
        if (key.size() > 0 && is_type<Object>()) {
            let index = key.at(0);
            let next = (*this)[index];
            return next.get({key.begin() + 1, key.end()});

        } else if (key.size() > 0 && is_type<Array>()) {
            size_t const index = std::stoll(key.at(0));
            let next = (*this)[index];
            return next.get({key.begin() + 1, key.end()});

        } else if (key.size() > 0) {
            BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("type %s does not support get() with '%s'")
                % type_name() % key.at(0)).str())
            );
        } else {
            return *this;
        }
    }


    /*! Return a string representation of the value.
     * \return a string representing the value.
     */
    std::string string() const {
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
        } else if (is_type<wsRGBA>()) {
            return value<wsRGBA>().string();
        } else if (is_type<std::string>()) {
            return "\"" + value<std::string>() + "\"";
        } else if (is_type<boost::filesystem::path>()) {
            return "\"" + value<boost::filesystem::path>().string() + "\"";
        } else if (is_type<Array>()) {
            std::string s = "[";
            auto first = true;
            for (let &x: value<Array>()) {
                if (!first) {
                    s += ",";
                }
                s += x->string();
                first = false;
            }
            return s + "]";
        } else if (is_type<Object>()) {
            std::string s = "{";
            auto first = true;
            for (let &[k, v]: value<Object>()) {
                if (!v->is_type<Undefined>()) {
                    if (!first) {
                        s += ",";
                    }
                    s += k + ":" + v->string();
                    first = false;
                }
            }
            return s + "}";
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("type %s does not implement .string()") %
            type_name()).str())
        );
    }

    /*! Return the internal value as a std::any
     * \return the std::any value
     */
    std::any any() const {
        if (is_type<Array>()) {
            std::vector<std::any> r;
            for (let &x: value<Array>()) {
                r.push_back(x->any());
            }
            return r;

        } else if (is_type<Object>()) {
            std::map<std::string, std::any> r;
            for (let &[k, v]: value<Object>()) {
                if (!v->is_type<Undefined>()) {
                    r[k] = v->any();
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
        } else if (is_type<wsRGBA>()) {
            return !value<wsRGBA>().isTransparent();
        } else if (is_type<std::string>()) {
            return value<std::string>().size() > 0;
        } else if (is_type<boost::filesystem::path>()) {
            return true;
        } else if (is_type<Array>()) {
            return value<Array>().size() > 0;
        } else if (is_type<Object>()) {
            return value<Object>().size() > 0;
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("type %s does not implement .boolean()") % type_name()).str()));
    }

    CompareResult cmp(Value const &other) const {
        if (is_type<std::string>() && other.is_type<std::string>()) {
            return compare(value<std::string>(), other.value<std::string>());

        } else if (is_type<Array>() && other.is_type<Array>()) {
            let &l = value<Array>();
            let &r = other.value<Array>();

            auto l_iterator = l.begin();
            auto r_iterator = r.begin();
            while (l_iterator != l.end() && r_iterator != r.end()) {
                switch ((*l_iterator)->cmp(*r_iterator)) {
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
            let &l = value<Object>();
            let &r = other.value<Object>();

            auto l_iterator = l.begin();
            auto r_iterator = r.begin();
            while (l_iterator != l.end() && r_iterator != r.end()) {
                switch (compare(l_iterator->first, r_iterator->first)) {
                case CompareResult::LOWER: return CompareResult::LOWER;
                case CompareResult::HIGHER: return CompareResult::HIGHER;
                case CompareResult::SAME:;
                }

                switch (l_iterator->second->cmp(*r_iterator->second)) {
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

        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot compare values of types %s and %s") %
            type_name() % other.type_name()).str())
        );
    }

    Value operator-() const {
        if (is_type<int64_t>()) {
            return -value<int64_t>();
        } else if (is_type<double>()) {
            return -value<double>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot make value of type %s negative") % type_name()).str()));
    }

    Value operator~() const {
        if (is_type<int64_t>()) {
            return ~value<int64_t>();
        } else if (is_type<bool>()) {
            return !value<bool>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot invert value of type %s") % type_name()).str()));
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
        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot multiple value of type %s with value of type %s") %
            type_name() % other.type_name()).str())
        );
    }

    Value operator/(Value const &other) const {
        if (is_type<double>() || other.is_type<double>()) {
            return value<double>() / other.value<double>();
        } else if (is_type<int64_t>() || other.is_type<int64_t>()) {
            return value<int64_t>() / other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot divide value of type %s with value of type %s") %
            type_name() % other.type_name()).str())
        );
    }

    Value operator%(Value const &other) const {
        if (is_type<double>() || other.is_type<double>()) {
            return fmod(value<double>(), other.value<double>());
        } else if (is_type<int64_t>() || other.is_type<int64_t>()) {
            return value<int64_t>() % other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot take modulo of value of type %s with value of type %s") %
            type_name() % other.type_name()).str())
        );
    }

    Value operator+(Value const &other) const {
        if (is_type<boost::filesystem::path>() || other.is_type<boost::filesystem::path>()) {
            return value<boost::filesystem::path>() / other.value<boost::filesystem::path>();
        } else if (is_type<std::string>() && other.is_type<std::string>()) {
            return value<std::string>() + other.value<std::string>();
        } else if (is_type<Array>() && other.is_type<Array>()) {
            Array r;
            for (let &x: value<Array>()) { r.push_back(x); }
            for (let &x: other.value<Array>()) { r.push_back(x); }
            return r;
        } else if (is_type<Object>() && other.is_type<Object>()) {
            Object r;
            for (let &[k, v]: other.value<Object>()) { r.emplace(k, v); }
            // emplace() will only insert if it doesn't exist.
            for (let &[k, v]: value<Object>()) { r.emplace(k, v); }
            return r;
        } else if (is_type<double>() || other.is_type<double>()) {
            return value<double>() + other.value<double>();
        } else if (is_type<int64_t>() || other.is_type<int64_t>()) {
            return value<int64_t>() + other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot add value of type %s to a value of type %s") %
            other.type_name() % type_name()).str())
        );
    }

    Value operator-(Value const &other) const {
        if (is_type<double>() || other.is_type<double>()) {
            return value<double>() - other.value<double>();
        } else if (is_type<int64_t>() || other.is_type<int64_t>()) {
            return value<int64_t>() - other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot subtract value of type %s from a value of type %s") %
            other.type_name() % type_name()).str())
        );
    }

    Value operator<<(Value const &other) const {
        if (is_type<int64_t>() && other.is_type<int64_t>()) {
            return value<int64_t>() << other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot left shift a of value of type %s with a value of type %s") %
            type_name() % other.type_name()).str())
        );
    }

    Value operator>>(Value const &other) const {
        if (is_type<int64_t>() && other.is_type<int64_t>()) {
            return value<int64_t>() >> other.value<int64_t>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot right shift a of value of type %s with a value of type %s") %
            type_name() % other.type_name()).str())
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
        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot binary-and a of value of type %s to a value of type %s") %
            other.type_name() % type_name() ).str())
        );
    }

    Value operator^(Value const &other) const {
        if (is_type<int64_t>() && other.is_type<int64_t>()) {
            return value<int64_t>() ^ other.value<int64_t>();
        } else if (is_type<bool>() && other.is_type<bool>()) {
            return static_cast<bool>(value<bool>() ^ other.value<bool>());
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot binary-xor a of value of type %s to a value of type %s") %
            other.type_name() % type_name() ).str())
        );
    }

    Value operator|(Value const &other) const {
        if (is_type<int64_t>() && other.is_type<int64_t>()) {
            return value<int64_t>() | other.value<int64_t>();
        } else if (is_type<bool>() && other.is_type<bool>()) {
            return value<bool>() || other.value<bool>();
        }
        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot binary-or a of value of type %s to a value of type %s") %
            other.type_name() % type_name() ).str())
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

    Value &operator[](std::string const &other) {
        if (is_type<Undefined>()) {
            // When accessing a name on an undefined it means we need replace it with an empty object.
            intrinsic = Object{};
        }

        if (is_type<Object>()) {
            auto &obj = value<Object>();
            auto [i, didInsert] = obj.try_emplace(other, Undefined{});
            return *(i->second);
        }

        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot get member .%s of type %s") %
            other % type_name()).str())
        );
    }

    Value operator[](std::string const &other) const {
        if (is_type<Object>()) {
            auto obj = value<Object>();
            auto i = obj.find(other);

            if (i != obj.end()) {
                return i->second;
            }
        }

        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot get member .%s of type %s") %
            other % type_name()).str())
        );
    }

    Value &operator[](size_t const index) {
        if (is_type<Undefined>()) {
            // When accessing a name on an undefined it means we need replace it with an empty object.
            intrinsic = Array{};
        }

        if (is_type<Array>()) {
            auto &_array = value<Array>();

            if (index < _array.size()) {
                return *(_array[index]);
            } else {
                BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Index %i out of range, size of array is %i") %
                    index % _array.size()).str())
                );
            }
        }

        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot get item at index %i of type %s") %
            index % type_name()).str())
        );
    }

    Value operator[](size_t const index) const {
        if (is_type<Array>()) {
            auto _array = value<Array>();
            
            if (index < _array.size()) {
                return *(_array[index]);
            } else {
                BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Index %i out of range, size of array is %i") %
                    index % _array.size()).str())
                );
            }
        }

        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot get item at index %i of type %s") %
            index % type_name()).str())
        );
    }

    Value &append() {
        if (is_type<Undefined>()) {
            // When accessing a name on an undefined it means we need replace it with an empty object.
            intrinsic = Array{};
        }

        if (is_type<Array>()) {
            auto &_array = value<Array>();
            _array.emplace_back(Undefined{});
            return *(_array.back());
        }

        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot append new item onto type %s") %
            type_name()).str())
        );
    }
};

}
