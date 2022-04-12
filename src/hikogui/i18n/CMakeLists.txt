
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

if(TT_BUILD_TESTS)
    target_sources(hikogui_tests PRIVATE
        iso_639_tests.cpp
        iso_3166_tests.cpp
        iso_15924_tests.cpp
        language_tag_tests.cpp
    )
endif()
