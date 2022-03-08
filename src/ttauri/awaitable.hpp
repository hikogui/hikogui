// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace tt::inline v1 {

template<typename Result>
class awaitable_base {
public:
    using result_type = Result;

protected:
    result_type _result;
};

template<>
class awaitable_base<void> {
public:
    using result_type = void;
};

template<typename Result, typename Schedular>
class awaitable {};

}