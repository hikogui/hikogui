
cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0054 NEW)
enable_testing()

project(TTauri LANGUAGES CXX)

#find_package(Doxygen)
find_package(GTest)
find_package(Python COMPONENTS Interpreter)
find_package(Vulkan REQUIRED)


set(BUILD_TTAURI_GUI OFF CACHE BOOL "Enable TTAURI GUI") 
set(BUILD_TTAURI_AUDIO OFF CACHE BOOL "Enable TTAURI Audio") 
set(BUILD_TTAURI_NET OFF CACHE BOOL "Enable TTAURI Networking") 

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake")
include(AddIcon)
include(AddStaticResource)
include(AddShader)
include(AddStaticResource)

add_subdirectory(external)
add_subdirectory(tools)

#
# Foundation
#
configure_file(
    src/TTauri/Foundation/config.hpp.in 
    src/TTauri/Foundation/config.hpp
)

set(TTauri_Foundation_srcs
    src/TTauri/Foundation/globals.cpp
    src/TTauri/Foundation/thread.cpp
    src/TTauri/Foundation/URL.cpp
    src/TTauri/Foundation/url_parser.cpp
    src/TTauri/Foundation/JSON.cpp
    src/TTauri/Foundation/globals.cpp
    src/TTauri/Foundation/hires_utc_clock.cpp
    src/TTauri/Foundation/logger.cpp
    src/TTauri/Foundation/trace.cpp
    src/TTauri/Foundation/StaticResourceView.cpp
    src/TTauri/Foundation/base93.cpp
    src/TTauri/Foundation/tokenizer.cpp
    src/TTauri/Foundation/BezierCurve.cpp
    src/TTauri/Foundation/Path.cpp
    src/TTauri/Foundation/PixelMap.cpp
    src/TTauri/Foundation/TTauriIconParser.cpp
    src/TTauri/Foundation/FileMapping.cpp
    src/TTauri/Foundation/FileView.cpp
    src/TTauri/Foundation/throw_exception.cpp
    src/TTauri/Foundation/expression.cpp
    src/TTauri/Foundation/inflate.cpp
    src/TTauri/Foundation/template.cpp
    src/TTauri/Foundation/gzip.cpp
    src/TTauri/Foundation/png.cpp
    src/TTauri/Foundation/vec.cpp
    src/TTauri/Foundation/zlib.cpp
)

if(WIN32)
    set(TTauri_Foundation_srcs ${TTauri_Foundation_srcs}
        src/TTauri/Foundation/URL_win32.cpp
        src/TTauri/Foundation/hires_utc_clock_win32.cpp
        src/TTauri/Foundation/audio_counter_clock_win32.cpp
        src/TTauri/Foundation/logger_win32.cpp
        src/TTauri/Foundation/File_win32.cpp
        src/TTauri/Foundation/FileMapping_win32.cpp
        src/TTauri/Foundation/FileView_win32.cpp
        src/TTauri/Foundation/debugger_win32.cpp
        src/TTauri/Foundation/fast_mutex_win32.cpp
    )
elseif(APPLE)
    set(TTauri_Foundation_srcs ${TTauri_Foundation_srcs}
        src/TTauri/Foundation/URL_macos.mm
        src/TTauri/Foundation/URL_posix.cpp
        src/TTauri/Foundation/hires_utc_clock_posix.cpp
        src/TTauri/Foundation/audio_counter_clock_macos.cpp
        src/TTauri/Foundation/logger_macos.mm
        src/TTauri/Foundation/File_posix.cpp
        src/TTauri/Foundation/FileMapping_posix.cpp
        src/TTauri/Foundation/FileView_posix.cpp
        src/TTauri/Foundation/debugger_macos.mm
        src/TTauri/Foundation/fast_mutex_macos.cpp
    )
endif()

set(TTauri_Foundation_tests
    src/TTauri/Foundation/polymorphic_value_tests.cpp
    src/TTauri/Foundation/string_tag_tests.cpp
    src/TTauri/Foundation/strings_tests.cpp
    src/TTauri/Foundation/url_parser_tests.cpp
    src/TTauri/Foundation/URL_tests.cpp
    src/TTauri/Foundation/bigint_tests.cpp
    src/TTauri/Foundation/int_overflow_tests.cpp
    src/TTauri/Foundation/counters_tests.cpp
    src/TTauri/Foundation/datum_tests.cpp
    src/TTauri/Foundation/exceptions_tests.cpp
    src/TTauri/Foundation/expression_tests.cpp
    src/TTauri/Foundation/FileView_tests.cpp
    src/TTauri/Foundation/pickle_tests.cpp
    src/TTauri/Foundation/polynomial_tests.cpp
    src/TTauri/Foundation/small_map_tests.cpp
    src/TTauri/Foundation/base93_tests.cpp
    src/TTauri/Foundation/safe_int_tests.cpp
    src/TTauri/Foundation/BezierCurve_tests.cpp
    src/TTauri/Foundation/Path_tests.cpp
    src/TTauri/Foundation/PixelMap_tests.cpp
    src/TTauri/Foundation/tokenizer_tests.cpp
    src/TTauri/Foundation/glob_tests.cpp
    src/TTauri/Foundation/JSON_tests.cpp
    src/TTauri/Foundation/decimal_tests.cpp
    src/TTauri/Foundation/template_tests.cpp
    src/TTauri/Foundation/math_tests.cpp
    src/TTauri/Foundation/TT5_tests.cpp
    src/TTauri/Foundation/vec_tests.cpp
    src/TTauri/Foundation/mat_tests.cpp
    src/TTauri/Foundation/Unicode_tests.cpp
    src/TTauri/Foundation/gzip_tests.cpp
)

add_library(TTauri_Foundation STATIC ${TTauri_Foundation_srcs})
target_link_libraries(TTauri_Foundation PUBLIC fmt tz span)
if (WIN32)
    target_link_libraries(TTauri_Foundation PUBLIC synchronization)
endif()
target_include_directories(TTauri_Foundation PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(TTauri_Foundation PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
target_include_directories(TTauri_Foundation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_property(TARGET TTauri_Foundation PROPERTY CXX_STANDARD 17)

add_custom_command(
  TARGET TTauri_Foundation POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_CURRENT_SOURCE_DIR}/external/tzdata 
    ${CMAKE_BINARY_DIR}/data/tzdata)

# 
# We will support the following CPUs:
#   * Intel Ivy Bridge from 2012, still used by Mac Pro sold in 2019.
#   * AMD Steamroller from 2014
#
#
if (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    # Clang will complain about __builtin_assume even on constexpr functions and operators,
    # this warning is absolutely useless.
    target_compile_options(TTauri_Foundation PUBLIC -Wno-assume)

    # Clang will complain about calling hidden-friend-template-functions.
    target_compile_options(TTauri_Foundation PUBLIC -Wno-unknown-warning-option -Wno-c++20-extensions -Wno-c++2a-extensions)

    # vk_mem_alloc.h seems to turn on nullability completeness checks on clang.
    # It seems this check should only be used for interopability with swift
    target_compile_options(TTauri_Foundation PUBLIC -Wno-nullability-completeness)

    # IvyBridge:                     F16C             FSGSBASE, AVX,           AES, PCLMUL        MMX, SSE, SSE2, SSE3,        SSSE3, SSE4.1, SSE4.2,     POPCNT, RDRND
    # Steamroller 0x15-v3: BMI, TBM, F16C, FMA, FMA4, FSGSBASE, AVX, XOP, LWP, AES, PCLMUL, CX16, MMX, SSE, SSE2, SSE3, SSE4A, SSSE3, SSE4.1, SSE4.2, ABM 
    # POPCNT is supported by Steamroller through he SSE4A instruction set.
    target_compile_options(TTauri_Foundation PUBLIC -mf16c -mfsgsbase -mavx -maes -mpclmul -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mpopcnt)

    # clang bug does not recognize functions without side-effects in __builtin_assume()
    target_compile_options(TTauri_Foundation PUBLIC -Wno-assume)

    # The Microsoft version of clang does not implement all clang command line arguments.
    if (NOT MSVC)
        # The TTauri generic tokenizer uses large constexpr tables.
        target_compile_options(TTauri_Foundation PUBLIC -fconstexpr-steps=100000000)

        # Tune according to a more modern CPU.
        target_compile_options(TTauri_Foundation PUBLIC -mtune=skylake)
    endif()

elseif (MSVC)
    # Ignore unknown pragmas.
    target_compile_options(TTauri_Foundation PUBLIC -wd4068)

    # Support Intel's IvyBridge and AMD's Steamroller
    target_compile_options(TTauri_Foundation PUBLIC -arch:AVX)

    # The TTauri generic tokenizer uses large constexpr tables.
    target_compile_options(TTauri_Foundation PUBLIC -constexpr:steps 100000000)

    target_compile_options(TTauri_Foundation PUBLIC -W4)
    #target_compile_options(TTauri_Foundation PUBLIC -permissive-)

    # C4324 structure was padded due to alignment specifier.
    # Ofcourse it is, that is what the alignment specifier is used for??????
    target_compile_options(TTauri_Foundation PUBLIC -wd4324)

    # C4100 unreferenced formal parameter.
    # This warning is in the way when you want to document a parameter of a virtual function
    # with an implementation that does not use the parameter.
    target_compile_options(TTauri_Foundation PUBLIC -wd4100)

    if (NOT ${CMAKE_BUILD_TYPE} MATCHES "Release")
        # Enable Security Development Lifecycle checks.
        # This includes run-time checks, don't include this in the Release type
        # Do include it with ReleaseWithDebug.
        target_compile_options(TTauri_Foundation PUBLIC -sdl)
    endif()
endif()

if (WIN32)
    # Set defines to compile a win32 application.
	target_compile_options(TTauri_Foundation PUBLIC -D_WIN64 -DUNICODE -D_UNICODE -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)

    # std::result_of used by fmt.
    target_compile_options(TTauri_Foundation PUBLIC -D_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING)

elseif (APPLE)
    target_link_libraries(TTauri_Foundation PUBLIC "-framework Foundation -framework AppKit")
endif()

set(TTauri_Foundation_test_files
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/file_view.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/includer.ttt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/includee.tti
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/gzip_test1.bin
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/gzip_test1.bin.gz
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/gzip_test2.bin
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/gzip_test2.bin.gz
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/gzip_test3.bin
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/gzip_test3.bin.gz
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/gzip_test4.bin
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/gzip_test4.bin.gz
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/gzip_test5.bin
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/gzip_test5.bin.gz
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/gzip_test6.bin
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/gzip_test6.bin.gz
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/gzip_test7.bin
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/gzip_test7.bin.gz
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/gzip_test8.bin
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/gzip_test8.bin.gz
)

add_executable(Foundation_tests ${TTauri_Foundation_tests})
target_include_directories(Foundation_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(Foundation_tests PRIVATE gtest_main TTauri_Foundation)
gtest_discover_tests(Foundation_tests)
add_custom_command(
    TARGET Foundation_tests PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${TTauri_Foundation_test_files} ${CMAKE_CURRENT_BINARY_DIR})


#
# Net
#
if(BUILD_TTAURI_NET)
message("TTauri Networking enabled")

set(TTauri_Net_srcs
    src/TTauri/Net/globals.cpp
)

add_library(TTauri_Net STATIC ${TTauri_Net_srcs})
target_link_libraries(TTauri_Net TTauri_Foundation)
target_include_directories(TTauri_Net PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
endif()

#
# Audio
#
if(BUILD_TTAURI_AUDIO)
set(TTauri_Audio_srcs
    src/TTauri/Audio/globals.cpp
    src/TTauri/Audio/AudioDevice.cpp
    src/TTauri/Audio/AudioSystem.cpp
)

if (WIN32)
set(TTauri_Audio_srcs
    ${TTauri_Audio_srcs}
    src/TTauri/Audio/AudioDevice_win32.cpp
    src/TTauri/Audio/AudioSystem_win32.cpp
)
elseif (APPLE)
set(TTauri_Audio_srcs
    ${TTauri_Audio_srcs}
)
endif()

add_library(TTauri_Audio STATIC ${TTauri_Audio_srcs})
target_link_libraries(TTauri_Audio TTauri_Foundation)
target_include_directories(TTauri_Audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
endif()


if(BUILD_TTAURI_GUI)
message("TTauri GUI enabled")

#
# Text
#
set(TTauri_Text_srcs
    src/TTauri/Text/globals.cpp
    src/TTauri/Text/UnicodeData.cpp
    src/TTauri/Text/UnicodeBidi.cpp
    src/TTauri/Text/UnicodeRanges.cpp
    src/TTauri/Text/Font.cpp
    src/TTauri/Text/TrueTypeFont.cpp
    src/TTauri/Text/FontBook.cpp
    src/TTauri/Text/Grapheme.cpp
    src/TTauri/Text/gstring.cpp
    src/TTauri/Text/ShapedText.cpp
    src/TTauri/Text/TextStyle.cpp
    src/TTauri/Text/FontGlyphIDs.cpp
    src/TTauri/Text/AttributedGlyph.cpp
)

set(UNICODE_DATA_CONVERTER_PY "${CMAKE_CURRENT_SOURCE_DIR}/tools/UnicodeData_converter.py")
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
target_include_directories(TTauri_Text PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(TTauri_Text_tests
    src/TTauri/Text/UnicodeData_tests.cpp
)

set(TTauri_Text_test_files
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/NormalizationTest.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/GraphemeBreakTest.txt
)

add_executable(Text_tests ${TTauri_Text_tests})
target_include_directories(Text_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(Text_tests PRIVATE gtest_main TTauri_Foundation TTauri_Text)
gtest_discover_tests(Text_tests)
add_custom_command(
    TARGET Text_tests PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${TTauri_Text_test_files} ${CMAKE_CURRENT_BINARY_DIR})


#
# GUI
#
add_shader(TTauri_GUI_Shader_objs
    src/TTauri/GUI/PipelineImage.vert
    src/TTauri/GUI/PipelineImage.frag
    src/TTauri/GUI/PipelineFlat.vert
    src/TTauri/GUI/PipelineFlat.frag
    src/TTauri/GUI/PipelineBox.vert
    src/TTauri/GUI/PipelineBox.frag
    src/TTauri/GUI/PipelineSDF.vert
    src/TTauri/GUI/PipelineSDF.frag
    src/TTauri/GUI/PipelineToneMapper.vert
    src/TTauri/GUI/PipelineToneMapper.frag
)

set(TTauri_GUI_data
    ${CMAKE_CURRENT_SOURCE_DIR}/data/win32.keybinds.json
)

set(TTauri_GUI_theme_data
    ${CMAKE_CURRENT_SOURCE_DIR}/data/themes/ttauri_dark.theme.json
    ${CMAKE_CURRENT_SOURCE_DIR}/data/themes/ttauri_light.theme.json
)

set(TTauri_GUI_srcs
    src/TTauri/GUI/globals.cpp
    src/TTauri/GUI/RenderDoc.cpp
    src/TTauri/GUI/GUIDevice_base.cpp
    src/TTauri/GUI/GUIDevice_vulkan.cpp
    src/TTauri/GUI/GUISystem_base.cpp
    src/TTauri/GUI/GUISystem_vulkan.cpp
    src/TTauri/GUI/Pipeline_base.cpp
    src/TTauri/GUI/Pipeline_vulkan.cpp
    src/TTauri/GUI/PipelineFlat.cpp
    src/TTauri/GUI/PipelineFlat_DeviceShared.cpp
    src/TTauri/GUI/PipelineToneMapper.cpp
    src/TTauri/GUI/PipelineToneMapper_DeviceShared.cpp
    src/TTauri/GUI/PipelineBox.cpp
    src/TTauri/GUI/PipelineBox_DeviceShared.cpp
    src/TTauri/GUI/PipelineImage.cpp
    src/TTauri/GUI/PipelineImage_DeviceShared.cpp
    src/TTauri/GUI/PipelineImage_Image.cpp
    src/TTauri/GUI/PipelineImage_TextureMap.cpp
    src/TTauri/GUI/PipelineSDF.cpp
    src/TTauri/GUI/PipelineSDF_DeviceShared.cpp
    src/TTauri/GUI/PipelineSDF_TextureMap.cpp
    src/TTauri/GUI/PipelineSDF_AtlasRect.cpp
    src/TTauri/GUI/VulkanMemoryAllocator.cpp
    src/TTauri/GUI/Window_base.cpp
    src/TTauri/GUI/Window_vulkan.cpp
    src/TTauri/GUI/Theme.cpp
    src/TTauri/GUI/ThemeBook.cpp
    src/TTauri/GUI/KeyboardBindings.cpp
    src/TTauri/GUI/KeyboardEvent.cpp
    src/TTauri/GUI/Image.cpp
)

if(WIN32)
set(TTauri_GUI_srcs
    ${TTauri_GUI_srcs}
    src/TTauri/GUI/GUISystem_vulkan_win32.cpp
    src/TTauri/GUI/ThemeMode_win32.cpp
    src/TTauri/GUI/Window_vulkan_win32.cpp
    src/TTauri/GUI/VerticalSync_win32.cpp
    src/TTauri/GUI/KeyboardVirtualKey_win32.cpp
)
elseif(APPLE)
set(TTauri_GUI_srcs
    ${TTauri_GUI_srcs}
    #src/TTauri/GUI/GUISystem_vulkan_macos.mm
    #src/TTauri/GUI/ThemeMode_macos.cpp
    #src/TTauri/GUI/Window_vulkan_macos.cpp
    #src/TTauri/GUI/VerticalSync_macos.cpp
)
endif()

set(TTauri_Widgets_srcs
    src/TTauri/Widgets/globals.cpp
    src/TTauri/Widgets/Widget.cpp
    src/TTauri/Widgets/WindowWidget.cpp
    src/TTauri/Widgets/ToolbarWidget.cpp
    src/TTauri/Widgets/ToolbarButtonWidget.cpp
    src/TTauri/Widgets/WindowTrafficLightsWidget.cpp
    src/TTauri/Widgets/ButtonWidget.cpp
    src/TTauri/Widgets/ToggleWidget.cpp
    src/TTauri/Widgets/LineInputWidget.cpp
    #src/TTauri/Widgets/ImageWidget.cpp
)

if(WIN32)
    set(TTauri_Widgets_srcs ${TTauri_Widgets_srcs}
        src/TTauri/Widgets/SystemMenuWidget.cpp
    )
endif()

add_library(TTauri_GUI STATIC ${TTauri_GUI_srcs} ${TTauri_Widgets_srcs})
target_static_resource(TTauri_GUI ${TTauri_GUI_Shader_objs})
target_link_libraries(TTauri_GUI TTauri_Foundation TTauri_Text ${Vulkan_LIBRARIES} VMA rhea RenderDoc)
target_include_directories(TTauri_GUI PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(TTauri_GUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${Vulkan_INCLUDE_DIRS})

if (WIN32)
    target_compile_definitions(TTauri_GUI PUBLIC -DVK_USE_PLATFORM_WIN32_KHR)
    target_link_libraries(TTauri_GUI UxTheme.lib)
elseif(APPLE)
    target_compile_definitions(TTauri_GUI PUBLIC -DVK_USE_PLATFORM_METAL_EXT)
endif()

add_custom_command(
  TARGET TTauri_GUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy 
    ${TTauri_GUI_data}
    ${CMAKE_BINARY_DIR}/data)

add_custom_command(
  TARGET TTauri_GUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory 
    ${CMAKE_BINARY_DIR}/data/themes)

add_custom_command(
  TARGET TTauri_GUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy 
    ${TTauri_GUI_theme_data}
    ${CMAKE_BINARY_DIR}/data/themes)

endif()


set(TTauri_Application_srcs
    src/TTauri/Application/Application_base.cpp
)

if(WIN32)
set(TTauri_Application_srcs ${TTauri_Application_srcs}
    src/TTauri/Application/Application_win32.cpp
)
elseif(APPLE)
set(TTauri_Application_srcs ${TTauri_Application_srcs}
    src/TTauri/Application/Application_macos.mm
)
endif()

set(TTauri_Application_Libraries TTauri_Foundation)
if(BUILD_TTAURI_NET)
message("tt::Application Networking Enabled")
set(TTauri_Application_Libraries ${TTauri_Application_Libraries} TTauri_Net)
endif()
if(BUILD_TTAURI_AUDIO)
message("tt::Application Audio Enabled")
set(TTauri_Application_Libraries ${TTauri_Application_Libraries} TTauri_Audio)
endif()
if(BUILD_TTAURI_GUI)
message("tt::Application GUI Enabled")
set(TTauri_Application_Libraries ${TTauri_Application_Libraries} TTauri_GUI)
endif()

add_library(TTauri_Application STATIC ${TTauri_Application_srcs})
target_link_libraries(TTauri_Application PUBLIC ${TTauri_Application_Libraries})
target_include_directories(TTauri_Application PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(TTauri INTERFACE)
target_link_libraries(TTauri INTERFACE TTauri_Application)




#doxygen_add_docs(TTauri_Doxygen
#    Foundation/include/
#)

#set(TTauri_do_not_analyze
#    ${TTauriTests_srcs}
#)

#if(MSVC)
#    add_compile_options(/wd4996 /wd4005)
#    if(TTAURI_ENABLE_CODE_ANALYSIS)
#        set_source_files_properties(${TTauri_do_not_analyze} PROPERTIES COMPILE_FLAGS /analyze-)
#        add_compile_options(/analyze /analyze:plugin EspXEngine.dll)
#        add_compile_options(/wd26447)
#       add_compile_options(/wd26486) # lifetime.3 tooooooo many false positives.
#   endif()
#endif()



