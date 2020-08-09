
find_package(Sphinx)

# configured documentation tools and intermediate build results
set(SPHINX_BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/docs")
 
# Sphinx cache with pickled ReST documents
set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/doctrees")
 
# HTML output directory
set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")
 
set(TTAURI_XML_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/../xml")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in"
    "${SPHINX_BINARY_BUILD_DIR}/conf.py"
    @ONLY)
 
add_custom_target(ttauri_sphinx ALL
    ${SPHINX_EXECUTABLE}
        -q -b html
        -c "${SPHINX_BINARY_BUILD_DIR}"
        -d "${SPHINX_CACHE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${SPHINX_HTML_DIR}"
    COMMENT "Building HTML documentation with Sphinx")

add_dependencies(ttauri_sphinx ttauri_doxygen)
