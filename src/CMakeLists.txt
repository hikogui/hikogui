
find_package(Doxygen)
find_package(Sphinx)

add_subdirectory(ttauri)

set(DOXYGEN_EXCLUDE_PATTERNS *.cpp)
set(DOXYGEN_GENERATE_HTML NO)
set(DOXYGEN_GENERATE_XML YES)
doxygen_add_docs(ttauri_doxygen ttauri)



# Sphinx cache with pickled ReST documents
set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/doctrees")
 
# HTML output directory
set(SPHINX_HTML_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/../html")
 
set(SPHINX_XML_INPUT "${CMAKE_CURRENT_BINARY_DIR}/xml")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in"
    "${CMAKE_CURRENT_BINARY_DIR}/conf.py"
    @ONLY)
 
add_custom_target(ttauri_sphinx ALL
    ${SPHINX_EXECUTABLE}
        -q -b html
        -c "${CMAKE_CURRENT_BINARY_DIR}"
        -d "${SPHINX_CACHE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${SPHINX_HTML_OUTPUT}"
    COMMENT "Building HTML documentation with Sphinx")

add_dependencies(ttauri_sphinx ttauri_doxygen)
add_dependencies(ttauri ttauri_sphinx)

