#
# Versioning
#
# Usage:
#
#   include(Versioning.cmake)
#
# Description:
#
# The root folder of the project contains a `VERSION` file.
# This `VERSION` file contains a version string, e.g. `1.2.3`.
# This version string is changed by the maintainer manually,
# before building and tagging the next version for release.
# By using a standalone versioning file you don't have to touch CMakeLists.txt.
#
# The version string is tokenized into [major].[minor].[patch] tokens.
#
# Then additional versioning information is collected and added.
# This allows to add any static or dynamic strings to the version.
# A dynamic version string might come from the source-control-system, e.g. git.
# That enables you to add, e.g. the fetched short-hash of a git commit,
# the git branch name or a current timestamp as release date.
# You could add the static string, e.g. "dev-local", if is not a git repo.
#
# All the version strings are then inserted into the version template file,
# which uses variable replacement tags (@VAR@).
#
# Finally, the new version header file is written.
#

#-------------------------------------------------------------------
# Read Version from VERSION file
#-------------------------------------------------------------------

file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/../VERSION" version)

if(${version} MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
    set(TTAURI_PROJECT_VERSION ${version})
    message(VERBOSE "Read version " ${TTAURI_PROJECT_VERSION} " from VERSION file.: ${PROJECT_VERSION}")
else()
    message(FATAL_ERROR
        "No valid version string found.\nPlease ensure the VERSION file in "
        "the root folder contains a version string in Major.Minor.Patch format.")
endif()

#-------------------------------------------------------------------
# Tokenize version into major minor patch
#-------------------------------------------------------------------

string(REGEX REPLACE "([0-9]+).[0-9]+.[0-9]+" "\\1" TTAURI_MAJOR_VERSION ${TTAURI_PROJECT_VERSION})
string(REGEX REPLACE "[0-9]+.([0-9]+).[0-9]+" "\\1" TTAURI_MINOR_VERSION ${TTAURI_PROJECT_VERSION})
string(REGEX REPLACE "[0-9]+.[0-9]+.([0-9]+)" "\\1" TTAURI_PATCH_VERSION ${TTAURI_PROJECT_VERSION})

#-------------------------------------------------------------------
# Get dynamic versioning strings from git
#-------------------------------------------------------------------

# check if "git" is installed
find_program(GIT_TOOL git DOC "Git version control")
mark_as_advanced(GIT_TOOL)

# check if source folder is a git repository
find_file(GIT_DIR NAMES .git PATHS ${CMAKE_SOURCE_DIR} NO_DEFAULT_PATH)

if(GIT_TOOL AND GIT_DIR)
  # get git commit hash
  execute_process(
    COMMAND git rev-parse --short=7 HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(GIT_COMMIT_HASH ${GIT_COMMIT_HASH_RAW})

  # get current git branch
  execute_process(
    COMMAND git branch --show-current
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(GIT_BRANCH ${GIT_BRANCH_RAW})

  unset(GIT_DIR)
else()
  # not a git repo, possibly a source download
  set(GIT_COMMIT_HASH "")
endif()

if(GIT_COMMIT_HASH_RAW)
  set(TTAURI_PROJECT_VERSION_SHORT ${TTAURI_PROJECT_VERSION})
  set(TTAURI_PROJECT_VERSION ${TTAURI_PROJECT_VERSION}+build.${GIT_COMMIT_HASH})
else()
  # add static string to indicate a non-repo version build
  set(TTAURI_PROJECT_VERSION ${TTAURI_PROJECT_VERSION}+build.local)
endif()

string(TIMESTAMP RELEASE_DATE "%Y-%m-%d")

#-------------------------------------------------------------------
# Overview
#-------------------------------------------------------------------

message(STATUS "[ -- Versioning Overview -- ]")
message(STATUS "")
message(STATUS "[Version] TTAURI_PROJECT_VERSION -> " ${TTAURI_PROJECT_VERSION})
message(STATUS "[Version] TTAURI_PROJECT_VERSION_SHORT -> " ${TTAURI_PROJECT_VERSION_SHORT})
message(STATUS "[Version] TTAURI_PROJECT_MAJOR -> " ${TTAURI_MAJOR_VERSION})
message(STATUS "[Version] TTAURI_PROJECT_MINOR -> " ${TTAURI_MINOR_VERSION})
message(STATUS "[Version] TTAURI_PROJECT_PATCH -> " ${TTAURI_PATCH_VERSION})
message(STATUS "[Version] GIT_COMMIT_HASH -> " ${GIT_COMMIT_HASH})
message(STATUS "[Version] GIT_BRANCH -> " ${GIT_BRANCH})
message(STATUS "[Version] RELEASE_DATE -> " ${RELEASE_DATE})
message(STATUS "")

#-------------------------------------------------------------------
# Write version file
#-------------------------------------------------------------------

# Load the template file `version.hpp.in`.
# Replace any @VAR@ tags and write `version.hpp`.
configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/../src/ttauri/version.hpp.in
    ${CMAKE_CURRENT_LIST_DIR}/../src/ttauri/version.hpp
)
