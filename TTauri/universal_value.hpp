// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "indirect_value.hpp"
#include "exceptions.hpp"
#include "math.hpp"
#include "wsRGBA.hpp"
#include "URL.hpp"
#include <boost/format.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <variant>
#include <any>
#include <map>
#include <vector>
#include <cstdint>
#include <cmath>
#include <typeinfo>
#include <string>
#include <memory>

namespace TTauri {

struct universal_value;

struct Undefined {};

using Object = std::map<std::string, indirect_value<universal_value>>;
using Array = std::vector<indirect_value<universal_value>>;

template<typename T> constexpr bool holds_alternative(universal_value const &x) noexcept;
template<typename T> inline T const &get(universal_value const & v) noexcept;
template<typename T> inline T const &&get(universal_value const && v) noexcept;
template<typename T> inline T &get(universal_value & v) noexcept;
template<typename T> inline T &&get(universal_value && v) noexcept;
template<typename T> inline T get_and_promote(universal_value const & v);

/*! A generic value type which will handle intra type operations.
 */
struct universal_value {
    std::variant<std::monostate,bool,int64_t,double,std::string,URL,wsRGBA,Object,Array,Undefined> intrinsic;

    universal_value() = default;
    ~universal_value() = default;
    universal_value(const universal_value &) = default;
    universal_value(universal_value &&) = default;
    universal_value &operator=(const universal_value &) = default;
    universal_value &operator=(universal_value &&) = default;

    universal_value(bool value) noexcept : intrinsic(value) {}
    universal_value(int64_t value) noexcept : intrinsic(value) {}
    universal_value(double value) noexcept : intrinsic(value) {}
    universal_value(std::string value) noexcept : intrinsic(std::move(value)) {}
    universal_value(URL value) noexcept : intrinsic(std::move(value)) {}
    universal_value(wsRGBA value) noexcept : intrinsic(value) {}
    universal_value(Object value) noexcept : intrinsic(std::move(value)) {}
    universal_value(Array value) noexcept : intrinsic(std::move(value)) {}
    universal_value(Undefined value) noexcept : intrinsic(std::move(value)) {}

    universal_value &operator=(bool value) noexcept { intrinsic = value; return *this; }
    universal_value &operator=(int64_t value) noexcept { intrinsic = value; return *this; }
    universal_value &operator=(double value) noexcept { intrinsic = value; return *this; }
    universal_value &operator=(std::string const &value) noexcept { intrinsic = value; return *this; }
    universal_value &operator=(URL const &value) noexcept { intrinsic = value; return *this; }
    universal_value &operator=(wsRGBA const &value) noexcept { intrinsic = value; return *this; }
    universal_value &operator=(Object const &value) noexcept { intrinsic = value; return *this; }
    universal_value &operator=(Array const &value) noexcept { intrinsic = value; return *this; }
    universal_value &operator=(Undefined const &value) noexcept { intrinsic = value; return *this; }

    std::type_info const &type() const noexcept {
        switch (intrinsic.index()) {
        case 0: return typeid(void);
        case 1: return typeid(bool);
        case 2: return typeid(int64_t);
        case 3: return typeid(double);
        case 4: return typeid(std::string);
        case 5: return typeid(URL);
        case 6: return typeid(wsRGBA);
        case 7: return typeid(Object);
        case 8: return typeid(Array);
        case 9: return typeid(Undefined);
        default: no_default;
        }
    }

    
    std::string type_name() const noexcept {
        return type().name();
    }

    template<typename T>
    bool is_promotable_to() const noexcept {
        if (holds_alternative<T>(intrinsic)) {
            return true;
        }

        if constexpr (std::is_same_v<std::remove_const_t<T>, double>) {
            return holds_alternative<int64_t>(intrinsic);
        } else if constexpr (std::is_same_v<std::remove_const_t<T>, URL>) {
            return holds_alternative<std::string>(intrinsic);
        } else {
            return holds_alternative<T>(intrinsic);
        }
    }

    universal_value &get_by_path(std::vector<std::string> const &key) {
        if (key.size() > 0 && holds_alternative<Object>(*this)) {
            let index = key.at(0);
            auto &next = (*this)[index];
            let next_key = std::vector<std::string>{key.begin() + 1, key.end()};
            return next.get_by_path(next_key);

        } else if (key.size() > 0 && holds_alternative<Array>(*this)) {
            size_t const index = std::stoll(key.at(0));
            auto &next = (*this)[index];
            let next_key = std::vector<std::string>{key.begin() + 1, key.end()};
            return next.get_by_path(next_key);

        } else if (key.size() > 0) {
            BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("type %s does not support get() with '%s'")
                % type_name() % key.at(0)).str())
            );
        } else {
            return *this;
        }
    }

    universal_value get_by_path(std::vector<std::string> const &key) const {
        if (key.size() > 0 && holds_alternative<Object>(*this)) {
            let index = key.at(0);
            let next = (*this)[index];
            return next.get_by_path({key.begin() + 1, key.end()});

        } else if (key.size() > 0 && holds_alternative<Array>(*this)) {
            size_t const index = std::stoll(key.at(0));
            let next = (*this)[index];
            return next.get_by_path({key.begin() + 1, key.end()});

        } else if (key.size() > 0) {
            BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("type %s does not support get() with '%s'")
                % type_name() % key.at(0)).str())
            );
        } else {
            return *this;
        }
    }

    /*! Return the internal value as a std::any
     * \return the std::any value
     */
    
    std::any any() const noexcept {
        if (holds_alternative<Array>(*this)) {
            std::vector<std::any> r;
            for (let &x: get<Array>(*this)) {
                r.push_back(x->any());
            }
            return r;

        } else if (holds_alternative<Object>(*this)) {
            std::map<std::string, std::any> r;
            for (let &[k, v]: get<Object>(*this)) {
                if (!holds_alternative<Undefined>(v)) {
                    r[k] = v->any();
                }
            }
            return r;

        } else {
            return intrinsic;
        }
    }

    operator bool() const noexcept {
        switch (intrinsic.index()) {
        case 0: return false;
        case 1: return get<bool>(*this);
        case 2: return get<int64_t>(*this) != 0;
        case 3: return get<double>(*this) != 0.0;
        case 4: return get<std::string>(*this).size() > 0;
        case 5: return true;
        case 6: return !get<wsRGBA>(*this).isTransparent();
        case 7: return get<Object>(*this).size() > 0;
        case 8: return get<Array>(*this).size() > 0;
        case 9: return false;
        default: no_default;
        }
    }

    universal_value &operator[](std::string const &other) {
        if (holds_alternative<Undefined>(intrinsic)) {
            // When accessing a name on an undefined it means we need replace it with an empty object.
            intrinsic = Object{};
        }

        if (holds_alternative<Object>(intrinsic)) {
            auto &obj = get<Object>(intrinsic);
            auto [i, did_insert] = obj.try_emplace(other, Undefined{});
            return *(i->second);
        }

        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot get member .%s of type %s") %
            other % type_name()).str())
        );
    }

    universal_value operator[](std::string const &other) const {
        if (holds_alternative<Object>(intrinsic)) {
            auto obj = get<Object>(intrinsic);
            auto i = obj.find(other);

            if (i != obj.end()) {
                return i->second;
            }
        }

        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot get member .%s of type %s") %
            other % type_name()).str())
        );
    }

    universal_value &operator[](size_t const index) {
        if (holds_alternative<Undefined>(intrinsic)) {
            // When accessing a name on an undefined it means we need replace it with an empty object.
            intrinsic = Array{};
        }

        if (holds_alternative<Array>(intrinsic)) {
            auto &_array = get<Array>(intrinsic);

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

    universal_value operator[](size_t const index) const {
        if (holds_alternative<Array>(intrinsic)) {
            auto _array = get<Array>(intrinsic);
            
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

    universal_value &append() {
        if (holds_alternative<Undefined>(intrinsic)) {
            // When accessing a name on an undefined it means we need replace it with an empty object.
            intrinsic = Array{};
        }

        if (holds_alternative<Array>(intrinsic)) {
            auto &_array = get<Array>(intrinsic);
            _array.emplace_back(Undefined{});
            return *(_array.back());
        }

        BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot append new item onto type %s") %
            type_name()).str())
        );
    }
};

inline bool operator!(universal_value const &rhs) noexcept
{
    return !static_cast<bool>(rhs);
}

inline bool operator==(universal_value const &lhs, universal_value const &rhs) noexcept
{
    if (holds_alternative<std::string>(lhs) && holds_alternative<std::string>(rhs)) {
        return get<std::string>(lhs) == get<std::string>(rhs);
    } else if (holds_alternative<Array>(lhs) && holds_alternative<Array>(rhs)) {
        return get<Array>(lhs) == get<Array>(rhs);
    } else if (holds_alternative<Object>(lhs) && holds_alternative<Object>(rhs)) {
        return get<Object>(lhs) == get<Object>(rhs);
    } else if (holds_alternative<double>(lhs) && holds_alternative<double>(rhs)) {
        return get<double>(lhs) == get<double>(rhs);
    } else if (holds_alternative<int64_t>(lhs) && holds_alternative<int64_t>(rhs)) {
        return get<int64_t>(lhs) == get<int64_t>(rhs);
    } else if (holds_alternative<bool>(lhs) && holds_alternative<bool>(rhs)) {
        return get<bool>(lhs) == get<bool>(rhs);
    } else {
        return false;
    }
}

inline bool operator!=(universal_value const &lhs, universal_value const &rhs) noexcept
{
    return !(lhs == rhs);
}

inline bool operator<(universal_value const &lhs, universal_value const &rhs) noexcept
{
    if (holds_alternative<std::string>(lhs) && holds_alternative<std::string>(rhs)) {
        return get<std::string>(lhs) < get<std::string>(rhs);
    } else if (holds_alternative<Array>(lhs) && holds_alternative<Array>(rhs)) {
        return get<Array>(lhs) < get<Array>(rhs);
    } else if (holds_alternative<Object>(lhs) && holds_alternative<Object>(rhs)) {
        return get<Object>(lhs) < get<Object>(rhs);
    } else if (holds_alternative<double>(lhs) || holds_alternative<double>(rhs)) {
        return get_and_promote<double>(lhs) < get_and_promote<double>(rhs);
    } else if (holds_alternative<int64_t>(lhs) || holds_alternative<int64_t>(rhs)) {
        return get<int64_t>(lhs) < get<int64_t>(rhs);
    } else if (holds_alternative<bool>(lhs) && holds_alternative<bool>(rhs)) {
        return get<bool>(lhs) < get<bool>(rhs);
    } else {
        // XXX Implement less-than based on type.
        return false;
    }
}

inline bool operator>(universal_value const &lhs, universal_value const &rhs) noexcept
{
    return rhs < lhs;
}

inline bool operator>=(universal_value const &lhs, universal_value const &rhs) noexcept
{
    return !(lhs < rhs);
}

inline bool operator<=(universal_value const &lhs, universal_value const &rhs) noexcept
{
    return !(lhs > rhs);
}

inline universal_value operator-(universal_value const &rhs)
{
    if (holds_alternative<int64_t>(rhs)) {
        return -get<int64_t>(rhs);
    } else if (holds_alternative<double>(rhs)) {
        return -get<double>(rhs);
    }
    BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot make value of type %s negative") % rhs.type_name()).str()));
}

inline universal_value operator~(universal_value const &rhs)
{
    if (holds_alternative<int64_t>(rhs)) {
        return ~get<int64_t>(rhs);
    } else if (holds_alternative<bool>(rhs)) {
        return !get<bool>(rhs);
    }
    BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot invert value of type %s") % rhs.type_name()).str()));
}

inline universal_value operator*(universal_value const &lhs, universal_value const &rhs)
{
    if (holds_alternative<double>(lhs) || holds_alternative<double>(rhs)) {
        return get_and_promote<double>(lhs) * get_and_promote<double>(rhs);
    } else if (holds_alternative<int64_t>(lhs) && holds_alternative<int64_t>(rhs)) {
        return get<int64_t>(lhs) * get<int64_t>(rhs);
    }
    BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot multiple value of type %s with value of type %s") %
        lhs.type_name() % rhs.type_name()).str())
    );
}

inline universal_value operator/(universal_value const &lhs, universal_value const &rhs)
{
    if (holds_alternative<double>(lhs) || holds_alternative<double>(rhs)) {
        return get_and_promote<double>(lhs) / get_and_promote<double>(rhs);
    } else if (holds_alternative<int64_t>(lhs) && holds_alternative<int64_t>(rhs)) {
        return get<int64_t>(lhs) / get<int64_t>(rhs);
    }
    BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot divide value of type %s with value of type %s") %
        lhs.type_name() % rhs.type_name()).str())
    );
}

inline universal_value operator%(universal_value const &lhs, universal_value const &rhs)
{
    if (holds_alternative<double>(lhs) || holds_alternative<double>(rhs)) {
        return fmod(get_and_promote<double>(lhs), get_and_promote<double>(rhs));
    } else if (holds_alternative<int64_t>(lhs) && holds_alternative<int64_t>(rhs)) {
        return modulo(get<int64_t>(lhs), get<int64_t>(rhs));
    }
    BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot take modulo of value of type %s with value of type %s") %
        lhs.type_name() % rhs.type_name()).str())
    );
}

inline universal_value operator+(universal_value const &lhs, universal_value const &rhs)
{
    if (holds_alternative<URL>(lhs) || holds_alternative<URL>(rhs)) {
        return get_and_promote<URL>(lhs) / get_and_promote<URL>(rhs);
    } else if (holds_alternative<std::string>(lhs) && holds_alternative<std::string>(rhs)) {
        return get<std::string>(lhs) + get<std::string>(rhs);
    } else if (holds_alternative<Array>(lhs) && holds_alternative<Array>(rhs)) {
        Array r;
        for (let &x: get<Array>(lhs)) { r.push_back(x); }
        for (let &x: get<Array>(rhs)) { r.push_back(x); }
        return r;
    } else if (holds_alternative<Object>(lhs) && holds_alternative<Object>(rhs)) {
        Object r;
        for (let &[k, v]: get<Object>(rhs)) { r.emplace(k, v); }
        // emplace() will only insert if it doesn't exist.
        for (let &[k, v]: get<Object>(lhs)) { r.emplace(k, v); }
        return r;
    } else if (holds_alternative<double>(lhs) || holds_alternative<double>(rhs)) {
        return get_and_promote<double>(lhs) + get_and_promote<double>(rhs);
    } else if (holds_alternative<int64_t>(lhs) && holds_alternative<int64_t>(rhs)) {
        return get<int64_t>(lhs) + get<int64_t>(rhs);
    }
    BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot add value of type %s to a value of type %s") %
        rhs.type_name() % lhs.type_name()).str())
    );
}

inline universal_value operator-(universal_value const &lhs, universal_value const &rhs)
{
    if (holds_alternative<double>(lhs) || holds_alternative<double>(rhs)) {
        return get_and_promote<double>(lhs) - get_and_promote<double>(rhs);
    } else if (holds_alternative<int64_t>(lhs) && holds_alternative<int64_t>(rhs)) {
        return get<int64_t>(lhs) - get<int64_t>(rhs);
    }
    BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot subtract value of type %s from a value of type %s") %
        rhs.type_name() % lhs.type_name()).str())
    );
}

inline universal_value operator<<(universal_value const &lhs, universal_value const &rhs)
{
    if (holds_alternative<int64_t>(lhs) && holds_alternative<int64_t>(rhs)) {
        return get<int64_t>(lhs) << get<int64_t>(rhs);
    }
    BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot left shift a of value of type %s with a value of type %s") %
        lhs.type_name() % rhs.type_name()).str())
    );
}

inline universal_value operator>>(universal_value const &lhs, universal_value const &rhs)
{
    if (holds_alternative<int64_t>(lhs) && holds_alternative<int64_t>(rhs)) {
        return get<int64_t>(lhs) >> get<int64_t>(rhs);
    }
    BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot right shift a of value of type %s with a value of type %s") %
        lhs.type_name() % rhs.type_name()).str())
    );
}

inline universal_value operator&(universal_value const &lhs, universal_value const &rhs)
{
    if (holds_alternative<int64_t>(lhs) && holds_alternative<int64_t>(rhs)) {
        return get<int64_t>(lhs) & get<int64_t>(rhs);
    } else if (holds_alternative<bool>(lhs) && holds_alternative<bool>(rhs)) {
        return get<bool>(lhs) && get<bool>(rhs);
    }
    BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot binary-and a of value of type %s to a value of type %s") %
        rhs.type_name() % lhs.type_name() ).str())
    );
}

inline universal_value operator^(universal_value const &lhs, universal_value const &rhs)
{
    if (holds_alternative<int64_t>(lhs) && holds_alternative<int64_t>(rhs)) {
        return get<int64_t>(lhs) ^ get<int64_t>(rhs);
    } else if (holds_alternative<bool>(lhs) && holds_alternative<bool>(rhs)) {
        return static_cast<bool>(get<bool>(lhs) ^ get<bool>(rhs));
    }
    BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot binary-xor a of value of type %s to a value of type %s") %
        rhs.type_name() % lhs.type_name() ).str())
    );
}

inline universal_value operator|(universal_value const &lhs, universal_value const &rhs)
{
    if (holds_alternative<int64_t>(lhs) && holds_alternative<int64_t>(rhs)) {
        return get<int64_t>(lhs) | get<int64_t>(rhs);
    } else if (holds_alternative<bool>(lhs) && holds_alternative<bool>(rhs)) {
        return get<bool>(lhs) || get<bool>(rhs);
    }
    BOOST_THROW_EXCEPTION(InvalidOperationError((boost::format("Cannot binary-or a of value of type %s to a value of type %s") %
        rhs.type_name() % lhs.type_name() ).str())
    );
}

template<typename T>
constexpr bool holds_alternative(universal_value const &x) noexcept
{
    return std::holds_alternative<T>(x.intrinsic);
}

template<typename T>
inline T const &get(universal_value const & v) noexcept
{
    return std::get<T>(v.intrinsic);
}

template<typename T>
inline T const &&get(universal_value const && v) noexcept
{
    return std::get<T>(v.intrinsic);
}

template<typename T>
inline T &get(universal_value & v) noexcept
{
    return std::get<T>(v.intrinsic);
}

template<typename T>
inline T &&get(universal_value && v) noexcept
{
    return std::get<T>(v.intrinsic);
}

template<typename T>
inline T get_and_promote(universal_value const& v)
{
    if constexpr (std::is_same_v<std::remove_const_t<T>, URL>) {
        if (holds_alternative<std::string>(v)) {
            return URL{get<std::string>(v)};
        } else {
            return get<T>(v);
        }

    } else if constexpr (std::is_same_v<std::remove_const_t<T>, double>) {
        if (holds_alternative<int64_t>(v)) {
            return static_cast<double>(get<int64_t>(v));
        } else {
            return get<T>(v);
        }

    } else {
        return get<T>(v);
    }
}

/*! Return a string representation of the value.
* \return a string representing the value.
*/
inline std::string to_string(universal_value const &x) noexcept
{
    switch (x.intrinsic.index()) {
    case 0:
        return "null";

    case 1:
        return get<bool>(x) ? "true" : "false";

    case 2:
        return std::to_string(get<int64_t>(x));

    case 3: {
            auto s = (boost::format("%g") % get<double>(x)).str();
            if (s.find('.') == s.npos) {
                return s + ".";
            } else {
                return s;
            }
        }

    case 4:
        return "\"" + get<std::string>(x) + "\"";

    case 5:
        return "\"" + to_string(get<URL>(x)) + "\"";

    case 6:
        return to_string(get<wsRGBA>(x));

    case 7: {
            std::string s = "{";
            auto first = true;
            for (let &[k, v]: get<Object>(x)) {
                if (!holds_alternative<Undefined>(v)) {
                    if (!first) {
                        s += ",";
                    }
                    s += k + ":" + to_string(v);
                    first = false;
                }
            }
            return s + "}";
        }

    case 8: {
            std::string s = "[";
            auto first = true;
            for (let &v: get<Array>(x)) {
                if (!first) {
                    s += ",";
                }
                s += to_string(v);
                first = false;
            }
            return s + "]";
        }

    case 9:
        return "Undefined";

    default:
        no_default;
    }
}

}
