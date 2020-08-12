
target_sources(ttauri PRIVATE
    veer.cpp
    veer.hpp
    veer_block_node.hpp
    veer_break_node.hpp
    veer_continue_node.hpp
    veer_do_node.hpp
    veer_expression_node.hpp
    veer_for_node.hpp
    veer_function_node.hpp
    veer_if_node.hpp
    veer_node.hpp
    veer_parse_context.cpp
    veer_parse_context.hpp
    veer_placeholder_node.hpp
    veer_return_node.hpp
    veer_string_node.hpp
    veer_top_node.hpp
    veer_while_node.hpp
)

target_sources(ttauri_tests PRIVATE
    veer_tests.cpp
)
