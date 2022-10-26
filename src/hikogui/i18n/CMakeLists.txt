# Copyright Take Vos 2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(hikogui PRIVATE
    iso_15924.cpp    
    iso_15924.hpp
    iso_639.hpp
	iso_3166.cpp
    iso_3166.hpp
    language.cpp
    language.hpp
	language_tag.cpp
	language_tag.hpp
    po_parser.cpp
    po_parser.hpp
	translate.hpp
    translation.cpp
    translation.hpp
)

if(BUILD_TESTING)
    target_sources(hikogui_tests PRIVATE
        iso_639_tests.cpp
        iso_3166_tests.cpp
        iso_15924_tests.cpp
        language_tag_tests.cpp
    )
endif()
