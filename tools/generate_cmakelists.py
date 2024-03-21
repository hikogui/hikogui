#!/usr/bin/python3

import sys
import os
import os.path

def get_header_files():
    header_files = []
    for dirpath, dirnames, filenames in os.walk("src"):
        for filename in filenames:
            path = os.path.join(dirpath, filename)
            if path.endswith(".hpp") or path.endswith(".h"):
                header_files.append(path)

    header_files.sort()
    return header_files

def make_cmakelist_includes_text(header_files, suppressed_header_files):
    r  = "# This file was generated with tools/generate_cmakelists.sh\n"
    r += "\n"
    r += "if(VCPKG_CHAINLOAD_TOOLCHAIN_FILE)\n"
    r += "    # vcpkg does not allow absolute paths anywhere in the install directory.\n"
    r += "    # These directories are normally used to execute files in their build\n"
    r += "    # directory; which does not happen on a vcpkg install.\n"
    r += "    set(LIBRARY_SOURCE_DIR \"vcpkg_no_source_dir\")\n"
    r += "    set(LIBRARY_BUILD_DIR \"vcpkg_no_build_dir\")\n"
    r += "else()\n"
    r += "    set(LIBRARY_SOURCE_DIR \"${CMAKE_CURRENT_SOURCE_DIR}\")\n"
    r += "    set(LIBRARY_BUILD_DIR \"${CMAKE_CURRENT_BINARY_DIR}\")\n"
    r += "endif()\n"
    r += "\n"
    r += "configure_file(\n"
    r += "    ${CMAKE_CURRENT_SOURCE_DIR}/src/hikogui/metadata/library_metadata.hpp.in\n"
    r += "    ${CMAKE_CURRENT_BINARY_DIR}/src/hikogui/metadata/library_metadata.hpp @ONLY)\n"
    r += "\n"
    r += "target_sources(hikogui INTERFACE FILE_SET hikogui_generated_include_files TYPE HEADERS BASE_DIRS \"${CMAKE_CURRENT_BINARY_DIR}/src/\" FILES\n"
    r += "    ${CMAKE_CURRENT_BINARY_DIR}/src/hikogui/metadata/library_metadata.hpp\n"
    r += ")\n"
    r += "\n"
    r += "target_sources(hikogui INTERFACE FILE_SET hikogui_include_files TYPE HEADERS BASE_DIRS \"${CMAKE_CURRENT_SOURCE_DIR}/src/\"  FILES\n"
    for header_file in header_files:
        if header_file.endswith("_win32.hpp") or header_file.endswith("_win32_impl.hpp") or header_file.endswith("_win32_intf.hpp"):
            r += "    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/%s>\n" % header_file.replace("\\", "/")

        if header_file.endswith("_posix.hpp") or header_file.endswith("_posix_impl.hpp") or header_file.endswith("_posix_intf.hpp"):
            r += "    $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_SOURCE_DIR}/%s>\n" % header_file.replace("\\", "/")

        if header_file.endswith("_macos.hpp") or header_file.endswith("_macos_impl.hpp") or header_file.endswith("_macos_intf.hpp"):
            r += "    $<$<PLATFORM_ID:MacOS>:${CMAKE_CURRENT_SOURCE_DIR}/%s>\n" % header_file.replace("\\", "/")

        if header_file.endswith("_x86.hpp") or header_file.endswith("_x86_impl.hpp") or header_file.endswith("_x86_intf.hpp"):
            r += "    $<$<STREQUAL:${ARCHITECTURE_ID},x86>:${CMAKE_CURRENT_SOURCE_DIR}/%s>\n" % header_file.replace("\\", "/")

        if header_file.endswith("_generic.hpp") or header_file.endswith("_generic_impl.hpp") or header_file.endswith("_generic_iintf.hpp"):
            r += "    $<$<STREQUAL:${ARCHITECTURE_ID},none>:${CMAKE_CURRENT_SOURCE_DIR}/%s>\n" % header_file.replace("\\", "/")

        if header_file in suppressed_header_files:
            r += "    #%s\n" % header_file.replace("\\", "/")

        else:
            r += "    %s\n" % header_file.replace("\\", "/")

    r += ")\n"
    r += "\n"
    return r

def generate_cmakelists_includes():
    header_files = get_header_files()
    suppressed_header_files = [
        "src/hikogui/net/packet_buffer.hpp"
        "src/hikogui/net/stream.hpp"
    ]

    with open("CMakeLists_includes.cmake", "w") as fd:
        fd.write(make_cmakelist_includes_text(header_files, suppressed_header_files))

def main():
    generate_cmakelists_includes()
    
    

if __name__ == "__main__":
    main()
