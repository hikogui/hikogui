
target_sources(ttauri PRIVATE
	expand_language_tag.cpp
	expand_language_tag.hpp
    iso_15924.cpp    
    iso_15924.hpp
    iso_639.hpp
	iso_3166.cpp
    iso_3166.hpp
	language_tag.cpp
	language_tag.hpp
)

if(TT_BUILD_TESTS)
    target_sources(ttauri_tests PRIVATE
        iso_15924_tests.cpp
        language_tag_tests.cpp
    )
endif()
