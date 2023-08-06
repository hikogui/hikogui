#
# Doxygen
#
find_package(Doxygen)

if(DOXYGEN_FOUND)
    set(DOXYGEN_EXCLUDE_PATTERNS *_tests.cpp *_impl.cpp *.mm)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_QUIET YES)
    set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
    set(DOXYGEN_WARN_NO_PARAMDOC NO)
    set(DOXYGEN_STRIP_FROM_PATH src)
    set(DOXYGEN_STRIP_FROM_INC_PATH src)

    # Use SHORT_NAMES to prevent Doxygen from generating filenames with double-quotes.
    # ALLOW_UNICODE_NAMES does not escape double-quotes.
    set(DOXYGEN_SHORT_NAMES YES)

    set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
    set(DOXYGEN_ALWAYS_DETAILED_SEC YES)
    set(DOXYGEN_DISTRIBUTE_GROUP_DOC YES)

    set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
    set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
    #set(DOXYGEN_CLANG_ASSISTED_PARSING YES)
    #set(DOXYGEN_CLANG_OPTIONS "-std=c++20")

    set(DOXYGEN_TAGFILES "${CMAKE_SOURCE_DIR}/docs/media/style/cppreference-doxygen-web.tag.xml=http://en.cppreference.com/w/")
    set(DOXYGEN_IMAGE_PATH "${CMAKE_SOURCE_DIR}/src/" "${CMAKE_SOURCE_DIR}/docs/")
    set(DOXYGEN_EXAMPLE_PATH "${CMAKE_SOURCE_DIR}/examples/")

    set(DOXYGEN_LAYOUT_FILE "${CMAKE_SOURCE_DIR}/docs/media/style/DoxygenLayout.xml")
    set(DOXYGEN_HTML_COLORSTYLE_HUE 24)
    set(DOXYGEN_HTML_COLORSTYLE_SAT 150)
    set(DOXYGEN_HTML_COLORSTYLE_GAMMA 80)
    set(DOXYGEN_HTML_EXTRA_STYLESHEET "${CMAKE_SOURCE_DIR}/docs/media/style/customdoxygen.css")


    # The following 4 settings are to get protected members to behave as private.
    set(DOXYGEN_ENABLE_PREPROCESSING YES)
    set(DOXYGEN_MACRO_EXPANSION YES)
    set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
    set(DOXYGEN_PREDEFINED "protected=private")

    # https://cmake.org/cmake/help/latest/module/FindDoxygen.html#command:doxygen_add_docs
    doxygen_add_docs(docs
      src/hikogui
      docs
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      COMMENT "Generate Documentation"
    )
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share COMPONENT docs EXCLUDE_FROM_ALL)
else()
    message("Please install Doxygen to generate the documentation.")
endif()
