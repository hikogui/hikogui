// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <type_traits>
#include <string>
#include <string_view>

export module hikogui_utility_reflection;
import hikogui_utility_assert;
import hikogui_utility_debugger;
import hikogui_utility_exception;
import hikogui_utility_fixed_string;
import hikogui_utility_misc;

export namespace hi { inline namespace v1 {

/** A type that can be implicitly converted to any type.
 */
struct convertible_to_any {
    template<typename T>
    [[nodiscard]] constexpr operator T() const noexcept
    {
        return {};
    }
};

namespace detail {

template<typename T, typename... C>
[[nodiscard]] constexpr size_t count_data_members() noexcept
{
    static_assert(std::is_trivially_constructible_v<T>);

    // Try the largest possible number of data members first. i.e. depth-first recursive.
    if constexpr (sizeof...(C) < sizeof(T)) {
        if constexpr (constexpr auto next = count_data_members<T, C..., convertible_to_any>()) {
            // We found the number of data members, larger than the current size.
            return next;
        }
    }

    // Check if this number of arguments to the constructor is valid .
    if constexpr (requires { T{C{}...}; }) {
        // Yes, return the number of arguments.
        return sizeof...(C);
    } else {
        // Nope, return false.
        return 0;
    }
}

[[nodiscard]] constexpr std::string_view type_name_token(std::string_view str) noexcept
{
    for (auto i = 0_uz; i != str.size(); ++i) {
        hilet c = str[i];

        if (not((c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or (c >= '0' and c <= '9') or c == '_')) {
            // End of token.
            return str.substr(0, i + 1);
        }
    }

    return str;
}

[[nodiscard]] constexpr std::string type_name_short_hand(std::string_view type_name) noexcept
{
    if (type_name == "std::basic_string<char,std::char_traits<char>,std::allocator<char>>") {
        return "std::string";
    } else if (type_name == "std::basic_string<char,std::char_traits<char>,std::allocator<char>>&") {
        return "std::string&";
    } else if (type_name == "const std::basic_string<char,std::char_traits<char>,std::allocator<char>>&") {
        return "const std::string&";
    } else {
        return std::string{type_name};
    }
}

#if HI_COMPILER == HI_CC_MSVC
template<typename T>
[[nodiscard]] constexpr std::string type_name() noexcept
{
    // "std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > __cdecl hi::v1::type_name<T>(void)
    // noexcept"
    auto signature = std::string_view{__FUNCSIG__};

    // Extract the type passed to type_name().
    auto first = signature.find("type_name<");
    hi_assert(first != std::string_view::npos);
    first += 10;

    auto last = signature.rfind('>');
    hi_assert(last != std::string_view::npos);

    signature = signature.substr(first, last - first);

    auto r = std::string{};
    while (not signature.empty()) {
        if (signature.starts_with("class ")) {
            signature = signature.substr(6);
        } else if (signature.starts_with("struct ")) {
            signature = signature.substr(7);
        } else if (signature.starts_with(" >")) {
            r += '>';
            signature = signature.substr(2);
        } else if (signature.starts_with(" *")) {
            r += '*';
            signature = signature.substr(2);
        } else {
            auto token = type_name_token(signature);
            r += token;
            signature = signature.substr(token.size());
        }
    }

    return type_name_short_hand(r);
}
#elif HI_COMPILER == HI_CC_CLANG
template<typename T>
[[nodiscard]] constexpr std::string type_name() noexcept
{
    // "std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > __cdecl hi::v1::type_name<T>(void)
    // noexcept"
    auto signature = std::string_view{__PRETTY_FUNCTION__ };

    // Extract the type passed to type_name().
    auto first = signature.find("type_name<");
    hi_assert(first != std::string_view::npos);
    first += 10;

    auto last = signature.rfind('>');
    hi_assert(last != std::string_view::npos);

    signature = signature.substr(first, last - first);

    auto r = std::string{};
    while (not signature.empty()) {
        if (signature.starts_with("class ")) {
            signature = signature.substr(6);
        } else if (signature.starts_with("struct ")) {
            signature = signature.substr(7);
        } else if (signature.starts_with(" >")) {
            r += '>';
            signature = signature.substr(2);
        } else if (signature.starts_with(" *")) {
            r += '*';
            signature = signature.substr(2);
        } else {
            auto token = type_name_token(signature);
            r += token;
            signature = signature.substr(token.size());
        }
    }

    return type_name_short_hand(r);
}
#else
#error "type_name() not implemented for this compiler."
#endif

} // namespace detail

template<typename T>
struct number_of_data_members : std::integral_constant<size_t, detail::count_data_members<T>()> {};

template<typename T>
constexpr size_t number_of_data_members_v = number_of_data_members<T>::value;

template<size_t Index, typename Type>
constexpr decltype(auto) get_data_member(Type&& rhs) noexcept
{
#define HI_X_FORWARD(x) forward_like<Type>(x),

    constexpr auto number_of_members = number_of_data_members<std::remove_cvref_t<Type>>();

    if constexpr (number_of_members == 0) {
        hi_static_no_default();

        // clang-format off
#define HI_X(count, ...) \
    } else if constexpr (number_of_members == count) { \
        auto&& [__VA_ARGS__] = rhs; \
        return std::get<Index>(std::forward_as_tuple(hi_for_each(HI_X_FORWARD, __VA_ARGS__) 0));

    HI_X( 1, a)
    HI_X( 2, a,b)
    HI_X( 3, a,b,c)
    HI_X( 4, a,b,c,d)
    HI_X( 5, a,b,c,d,e)
    HI_X( 6, a,b,c,d,e,f)
    HI_X( 7, a,b,c,d,e,f,g)
    HI_X( 8, a,b,c,d,e,f,g,h)
    HI_X( 9, a,b,c,d,e,f,g,h,i)
    HI_X(10, a,b,c,d,e,f,g,h,i,j)
    HI_X(11, a,b,c,d,e,f,g,h,i,j,k)
    HI_X(12, a,b,c,d,e,f,g,h,i,j,k,l)
    HI_X(13, a,b,c,d,e,f,g,h,i,j,k,l,m)
    HI_X(14, a,b,c,d,e,f,g,h,i,j,k,l,m,n)
    HI_X(15, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o)
    HI_X(16, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
    HI_X(17, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q)
    HI_X(18, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r)
    HI_X(19, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s)
    HI_X(20, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t)
    HI_X(21, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u)
    HI_X(22, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v)
    HI_X(23, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w)
    HI_X(24, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x)
    HI_X(25, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y)
    HI_X(26, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z)
    HI_X(27, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A)
    HI_X(28, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B)
    HI_X(29, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C)
    HI_X(30, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D)
    HI_X(31, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E)
    HI_X(32, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F)
    HI_X(33, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G)
    HI_X(34, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H)
    HI_X(35, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I)
    HI_X(36, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J)
    HI_X(37, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K)
    HI_X(38, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L)
    HI_X(39, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M)
    HI_X(40, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N)
    HI_X(41, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O)
    HI_X(42, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P)
    HI_X(43, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q)
    HI_X(44, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R)
    HI_X(45, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S)
    HI_X(46, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T)
    HI_X(47, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U)
    HI_X(48, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V)
    HI_X(49, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W)
    HI_X(50, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X)
    HI_X(51, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y)
    HI_X(52, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z)
        // clang-format on

    } else {
        hi_static_not_implemented();
    }

#undef HI_X
#undef HI_X_FORWARD
}

#if HI_COMPILER == HI_CC_MSVC
template<typename T>
[[nodiscard]] constexpr auto type_name() noexcept
{
    return hi_to_fixed_string(detail::type_name<T>());
}
#endif

}} // namespace hi::v1
