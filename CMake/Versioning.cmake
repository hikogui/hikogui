
#
# Versioning
#
# Usage:
#
#   include(Versioning.cmake)
#

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

  # get date of the git commit
  execute_process(
    COMMAND git log -1 --format=%cd --date=short
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_DATE_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(RELEASE_DATE ${GIT_COMMIT_DATE_RAW})

  # get long description of a git tag
  execute_process(
    COMMAND git describe --tags --abbrev=40 --dirty --broken --match=v[0-9][0-9.]* --long
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG_DESCRIPTION_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(GIT_TAG_RCS ${GIT_TAG_DESCRIPTION_RAW})

  unset(GIT_DIR)
else()
  # not a git repo, possibly a source download
  set(GIT_COMMIT_HASH "")
endif()

#-------------------------------------------------------------------
# Tokenize version into major, minor, patch, commits since tag
#-------------------------------------------------------------------

string(REGEX MATCH "v([0-9]+)\.([0-9]+)\.([0-9]+)\-([0-9]+)\-(.+)-(.+)" _ "${GIT_TAG_RCS}")
set(TTAURI_MAJOR_VERSION     ${CMAKE_MATCH_1})
set(TTAURI_MINOR_VERSION     ${CMAKE_MATCH_2})
set(TTAURI_PATCH_VERSION     ${CMAKE_MATCH_3})
set(GIT_COMMITS_SINCE_TAG    ${CMAKE_MATCH_4})
set(GIT_TAG_COMMIT_HASH      ${CMAKE_MATCH_5})
set(GIT_LOCAL_CHANGES        ${CMAKE_MATCH_6})

string(SUBSTRING ${GIT_TAG_COMMIT_HASH} 0 7 GIT_TAG_COMMIT_HASH_SHORT)

set(TTAURI_PROJECT_VERSION
"${TTAURI_MAJOR_VERSION}.${TTAURI_MINOR_VERSION}.${TTAURI_PATCH_VERSION}-${GIT_COMMITS_SINCE_TAG}-${GIT_TAG_COMMIT_HASH_SHORT}")

set(TTAURI_PROJECT_VERSION_SHORT "${TTAURI_MAJOR_VERSION}.${TTAURI_MINOR_VERSION}.${TTAURI_PATCH_VERSION}")

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
message(STATUS "[Version] GIT_TAG_RCS -> " ${GIT_TAG_RCS})
message(STATUS "[Version] GIT_COMMITS_SINCE_TAG  -> " ${GIT_COMMITS_SINCE_TAG})
message(STATUS "[Version] GIT_LOCAL_CHANGES -> " ${GIT_LOCAL_CHANGES})
message(STATUS "[Version] GIT_COMMIT_HASH -> " ${GIT_COMMIT_HASH})
message(STATUS "[Version] GIT_BRANCH -> " ${GIT_BRANCH})
message(STATUS "[Version] RELEASE_DATE (when committed) -> " ${RELEASE_DATE})
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
