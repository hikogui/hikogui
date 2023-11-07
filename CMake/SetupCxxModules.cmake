#-------------------------------------------------------------------
# Enable C++20 Modules
#-------------------------------------------------------------------
#
# Docs:     https://github.com/Kitware/CMake/blob/master/Help/dev/experimental.rst
# Issues:   https://gitlab.kitware.com/cmake/cmake/-/issues?label_name%5B%5D=area%3Acxxmodules
# Examples: https://gitlab.kitware.com/cmake/cmake/-/tree/master/Tests/RunCMake/CXXModules/examples


# Minimum Requirements: CMake 3.25+, MSVC 17.4+
cmake_minimum_required(VERSION 3.25)

# Enable C++ module support
if(${CMAKE_VERSION} VERSION_LESS "3.26")
  set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "3c375311-a3c9-4396-a187-3227ef642046")
elseif(${CMAKE_VERSION} VERSION_LESS "3.27")
  set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")
elseif(${CMAKE_VERSION} VERSION_LESS "3.28")
  set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "aa1f7df0-828a-4fcd-9afc-2dc80491aca7")
else()
  # 3.29 CXX_MODULE is no longer experimental.
endif()

# Turn on the dynamic dependencies for ninja
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
set(CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE 1)

# Indicate that Clang's modules support has trouble with language extensions.
if (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  if (DEFINED CMAKE_CXX_EXTENSIONS AND CMAKE_CXX_EXTENSIONS)
    message(WARNING
      "Clang's modules support has trouble with CMAKE_CXX_EXTENSIONS extensions.\n"
      "Please adjust your compiler setup: set(CMAKE_CXX_EXTENSIONS OFF)"
    )
  endif ()
endif()
