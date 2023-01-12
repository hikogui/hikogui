# Copyright Take Vos 2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(hikogui PRIVATE
    iso_15924_impl.cpp    
    iso_15924.hpp
    iso_639.hpp
	iso_3166_impl.cpp
    iso_3166.hpp
    language_impl.cpp
    language.hpp
	language_tag_impl.cpp
	language_tag.hpp
    po_parser_impl.cpp
    po_parser.hpp
	translate.hpp
    translation_impl.cpp
    translation.hpp
)
