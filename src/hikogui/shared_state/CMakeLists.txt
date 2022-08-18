# Copyright Take Vos 2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


target_sources(hikogui PRIVATE
    shared_state.hpp
    shared_state_cursor.hpp
)

if(HI_BUILD_TESTS)
    target_sources(hikogui_tests PRIVATE
        shared_state_tests.cpp
    )
endif()
