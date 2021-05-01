# Copyright Take Vos 2020.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(ttauri PRIVATE
    audio_block.hpp
    audio_device.cpp
    audio_device.hpp
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/audio_device_asio.hpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/audio_device_win32.cpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/audio_device_win32.hpp>
    audio_device_delegate.hpp
    audio_stream_config.hpp
    audio_system.cpp
    audio_system.hpp
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/audio_system_asio.hpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/audio_system_win32.cpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/audio_system_win32.hpp>
    audio_system_delegate.hpp
    audio_sample_unpacker.cpp
    audio_sample_unpacker.hpp
    #audio_sample_packer.cpp
    #audio_sample_packer.hpp
)

if(TT_BUILD_TESTS)
    target_sources(ttauri_tests PRIVATE
        audio_sample_unpacker_tests.cpp
    )
endif()

