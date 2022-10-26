# Copyright Take Vos 2020.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(hikogui PRIVATE
    skeleton.cpp
    skeleton.hpp
    skeleton_block_node.hpp
    skeleton_break_node.hpp
    skeleton_continue_node.hpp
    skeleton_do_node.hpp
    skeleton_expression_node.hpp
    skeleton_for_node.hpp
    skeleton_function_node.hpp
    skeleton_if_node.hpp
    skeleton_node.hpp
    skeleton_parse_context.cpp
    skeleton_parse_context.hpp
    skeleton_placeholder_node.hpp
    skeleton_return_node.hpp
    skeleton_string_node.hpp
    skeleton_top_node.hpp
    skeleton_while_node.hpp
)

if(BUILD_TESTING)
    target_sources(hikogui_tests PRIVATE
        skeleton_tests.cpp
    )
endif()
