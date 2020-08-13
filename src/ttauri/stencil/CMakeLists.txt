
target_sources(ttauri PRIVATE
    stencil.cpp
    stencil.hpp
    stencil_block_node.hpp
    stencil_break_node.hpp
    stencil_continue_node.hpp
    stencil_do_node.hpp
    stencil_expression_node.hpp
    stencil_for_node.hpp
    stencil_function_node.hpp
    stencil_if_node.hpp
    stencil_node.hpp
    stencil_parse_context.cpp
    stencil_parse_context.hpp
    stencil_placeholder_node.hpp
    stencil_return_node.hpp
    stencil_string_node.hpp
    stencil_top_node.hpp
    stencil_while_node.hpp
)

target_sources(ttauri_tests PRIVATE
    stencil_tests.cpp
)
