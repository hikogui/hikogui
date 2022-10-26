# Copyright Take Vos 2020.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(hikogui PRIVATE
    formula.cpp
    formula.hpp
    formula_add_node.hpp
    formula_arguments.hpp
    formula_assign_node.hpp
    formula_binary_operator_node.hpp
    formula_bit_and_node.hpp
    formula_bit_or_node.hpp
    formula_bit_xor_node.hpp
    formula_call_node.hpp
    formula_decrement_node.hpp
    formula_div_node.hpp
    formula_eq_node.hpp
    formula_evaluation_context.hpp
    formula_filter_node.hpp
    formula_ge_node.hpp
    formula_gt_node.hpp
    formula_increment_node.hpp
    formula_index_node.hpp
    formula_inplace_add_node.hpp
    formula_inplace_and_node.hpp
    formula_inplace_div_node.hpp
    formula_inplace_mod_node.hpp
    formula_inplace_mul_node.hpp
    formula_inplace_or_node.hpp
    formula_inplace_shl_node.hpp
    formula_inplace_shr_node.hpp
    formula_inplace_sub_node.hpp
    formula_inplace_xor_node.hpp
    formula_invert_node.hpp
    formula_le_node.hpp
    formula_literal_node.hpp
    formula_logical_and_node.hpp
    formula_logical_not_node.hpp
    formula_logical_or_node.hpp
    formula_lt_node.hpp
    formula_map_literal_node.hpp
    formula_member_node.hpp
    formula_minus_node.hpp
    formula_mod_node.hpp
    formula_mul_node.hpp
    formula_name_node.hpp
    formula_ne_node.hpp
    formula_node.hpp
    formula_parse_context.hpp
    formula_plus_node.hpp
    formula_post_process_context.cpp
    formula_post_process_context.hpp
    formula_pow_node.hpp
    formula_shl_node.hpp
    formula_shr_node.hpp
    formula_sub_node.hpp
    formula_ternary_operator_node.hpp
    formula_unary_operator_node.hpp
    formula_vector_literal_node.hpp
)

if(BUILD_TESTING)
    target_sources(hikogui_tests PRIVATE
        formula_tests.cpp
    )
endif()
