#
# SetupVcpkg
#
# This configures vcpkg using environment variables instead of a command-line options.
#
# https://github.com/microsoft/vcpkg/blob/master/docs/users/integration.md#using-an-environment-variable-instead-of-a-command-line-option
#
# Environment Variables: https://vcpkg.readthedocs.io/en/latest/users/config-environment/
#

set(VCPKG_VERBOSE ON)

# X_VCPKG_APPLOCAL_DEPS_INSTALL depends on CMake policy CMP0087
if(POLICY CMP0087)
    cmake_policy(SET CMP0087 NEW)
endif()

#
# Install vcpkg dependencies automatically.
#

# Copy dependencies into the output directory for executables.
set(VCPKG_APPLOCAL_DEPS ON)

# Copy dependencies into the install target directory for executables.
set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)

# == VCPKG_ROOT
#
# Please set VCPKG_ROOT on your env: export VCPKG_ROOT=/opt/vcpkg/bin
# This avoids passing it on the configure line: -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
#
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# == VCPKG_FEATURE_FLAGS
#
# This env var can be set to a comma-separated list of off-by-default features in vcpkg.
#
# Available features are: manifests, binarycaching.
#
# manifests     -> use the project-local manifest file "vcpkg.json" to build dependencies
# binarycaching -> use prebuild packages from cache to avoid rebuilds
#
# https://vcpkg.readthedocs.io/en/latest/specifications/manifests/
# https://vcpkg.readthedocs.io/en/latest/specifications/binarycaching/
#
if(NOT DEFINED ENV{VCPKG_FEATURE_FLAGS})
    set(ENV{VCPKG_FEATURE_FLAGS} "manifests")
endif()

# == VCPKG_TARGET_TRIPLET
#
# A triplet defines the build target environment in a compact string.
# [target-architecture]-[platform]-[linkage type]
# Examples: x86-windows, x64-windows-static, x64-linux.
#
# https://vcpkg.readthedocs.io/en/latest/users/triplets/
#
# The VCPKG_DEFAULT_TRIPLET is automatically set by vcpkg.cmake.
# The default triplets are: Windows: x86-windows, Linux: x64-linux, OSX: x64-osx.
#
# If you want to build for other platforms, e.g. build for Linux on Windows-x64 (canadian-cross builds)
# please set VCPKG_TARGET_TRIPLET as env var: export VCPKG_TARGET_TRIPLET=x64-linux
#
if(DEFINED ENV{VCPKG_TARGET_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_TARGET_TRIPLET}" CACHE STRING "")
endif()

# == VCPKG_DEFAULT_TRIPLET
#
if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
  set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

# == VCPKG_DIR
#
# VCPKG_DIR is the root folder for all compiled packages, e.g.
# the local /project/vcpkg_installed/x64-windows
# or the global /opt/vcpkg/installed/x64-windows.
#
# Because finding dependencies automatically is still on the todo list of vcpkg, we need to guide it.
#
# Please use this variable to point to the locations of your packages share folder like below:
# set(spdlog_DIR "${VCPKG_DIR}/share/spdlog")
# find_package(spdlog CONFIG REQUIRED)
#
# TODO: define VCPKG_DIR for globally installed packages (in the vcpkg root)
#
if(NOT DEFINED VCPKG_DIR)
    if(WIN32)
        set(VCPKG_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")
    else()
        set(VCPKG_DIR "${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")
    endif()
endif()

iF(NOT DEFINED VCPKG_MANIFEST_FILE)
    set(VCPKG_MANIFEST_FILE "${CMAKE_SOURCE_DIR}/vcpkg.json")
endif()

# Add this file and the VCPKG_MANIFEST_FILE as a "vcpkg" source_group to the IDE.
# They are not automatically picked up and listed as "important project" files by IDEs, yet.
source_group("vcpkg" FILES
    ${CMAKE_SOURCE_DIR}/cmake/SetupVcpkg.cmake
    ${CMAKE_SOURCE_DIR}/vcpkg.json
)

#
# Print VCPKG configuration overview
#
message(STATUS "")
message(STATUS "[VCPKG]  Configuration Overview:")
message(STATUS "")
message(STATUS "[VCPKG]  VCPKG_FEATURE_FLAGS     -> '$ENV{VCPKG_FEATURE_FLAGS}'")
message(STATUS "[VCPKG]  VCPKG_ROOT              -> '$ENV{VCPKG_ROOT}'")
message(STATUS "[VCPKG]  CMAKE_TOOLCHAIN_FILE    -> '${CMAKE_TOOLCHAIN_FILE}'")
message(STATUS "[VCPKG]  VCPKG_MANIFEST_FILE     -> '${VCPKG_MANIFEST_FILE}'")
message(STATUS "[VCPKG]  VCPKG_TARGET_TRIPLET    -> '${VCPKG_TARGET_TRIPLET}'")
message(STATUS "[VCPKG]  VCPKG_DIR               -> '${VCPKG_DIR}'")
message(STATUS "")
