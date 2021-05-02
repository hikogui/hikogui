# Copyright Take Vos 2020.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(ttauri PRIVATE
    pcg.hpp
    xorshift128p.hpp
)

if(TT_BUILD_TESTS)
    target_sources(ttauri_tests PRIVATE
        xorshift128p_tests.cpp
    )
endif()

