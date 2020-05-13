
project(TTauri_Text LANGUAGES)

include(GoogleTest)

set(TTauri_Text_srcs
    src/globals.cpp
    src/UnicodeData.cpp
    src/UnicodeBidi.cpp
    src/UnicodeRanges.cpp
    src/Font.cpp
    src/TrueTypeFont.cpp
    src/FontBook.cpp
    src/Grapheme.cpp
    src/gstring.cpp
    src/ShapedText.cpp
    src/TextStyle.cpp
    src/FontGlyphIDs.cpp
    src/AttributedGlyph.cpp
)

set(UNICODE_DATA_CONVERTER_PY "${CMAKE_CURRENT_SOURCE_DIR}/scripts/UnicodeData_converter.py")
set(UNICODE_DATA_TXT "${CMAKE_CURRENT_SOURCE_DIR}/data/UnicodeData.txt")
set(EMOJI_DATA_TXT "${CMAKE_CURRENT_SOURCE_DIR}/data/emoji-data.txt")
set(COMPOSITION_EXCLUSIONS_TXT "${CMAKE_CURRENT_SOURCE_DIR}/data/CompositionExclusions.txt")
set(GRAPHEME_BREAK_PROPERTY_TXT "${CMAKE_CURRENT_SOURCE_DIR}/data/GraphemeBreakProperty.txt")
set(UNICODE_DATA_BIN "${CMAKE_CURRENT_BINARY_DIR}/data/UnicodeData.bin")
get_filename_component(UNICODE_DATA_BIN_DIR ${UNICODE_DATA_BIN} DIRECTORY)
file(MAKE_DIRECTORY ${UNICODE_DATA_BIN_DIR})
add_custom_command(
	OUTPUT ${UNICODE_DATA_BIN}
	COMMAND ${Python_EXECUTABLE} ${UNICODE_DATA_CONVERTER_PY}
        --output=${UNICODE_DATA_BIN}
        --unicode-data=${UNICODE_DATA_TXT}
        --emoji-data=${EMOJI_DATA_TXT}
        --composition-exclusions=${COMPOSITION_EXCLUSIONS_TXT}
        --grapheme-break-property=${GRAPHEME_BREAK_PROPERTY_TXT}
	DEPENDS ${UNICODE_DATA_CONVERTER_PY} ${UNICODE_DATA_TXT} ${GRAPHEME_BREAK_PROPERTY_TXT}
	VERBATIM)

set(TTauri_Text_resources
    ${UNICODE_DATA_BIN}
    data/elusiveicons-webfont.ttf
    data/TTauriIcons.ttf
)

add_library(TTauri_Text STATIC ${TTauri_Text_srcs})
target_link_libraries(TTauri_Text TTauri_Foundation)
target_static_resource(TTauri_Text ${TTauri_Text_resources})
target_include_directories(TTauri_Text PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(TTauri_Text PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_definitions(TTauri_Text PUBLIC -D_WIN64)
endif()

set(TTauri_Text_tests
    tests/UnicodeData_tests.cpp
)

set(TTauri_Text_test_files
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/NormalizationTest.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/GraphemeBreakTest.txt
)

add_executable(Text_tests ${TTauri_Text_tests})
target_include_directories(Text_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(Text_tests PRIVATE gtest_main TTauri_Foundation TTauri_Text)
gtest_discover_tests(Text_tests)
add_custom_command(
    TARGET Text_tests PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${TTauri_Text_test_files} ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()
