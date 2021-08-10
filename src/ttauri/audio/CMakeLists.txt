# Copyright Take Vos 2020.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(ttauri PRIVATE
    audio_block.hpp
    audio_channel.hpp
    audio_device.cpp
    audio_device.hpp
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/audio_device_asio.hpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/audio_device_win32.cpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/audio_device_win32.hpp>
    audio_device_id.hpp
    audio_device_id.cpp
    audio_device_delegate.hpp
    audio_stream_config.hpp
    audio_system.cpp
    audio_system.hpp
    audio_system_aggregate.cpp
    audio_system_aggregate.hpp
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/audio_system_asio.hpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/audio_system_win32.cpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/audio_system_win32.hpp>
    audio_system_delegate.hpp
    audio_sample_format.cpp
    audio_sample_format.hpp
    audio_sample_unpacker.cpp
    audio_sample_unpacker.hpp
    audio_sample_packer.cpp
    audio_sample_packer.hpp
    pcm_format.hpp
    speaker_mapping.cpp
    speaker_mapping.hpp
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/speaker_mapping_win32.cpp>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/speaker_mapping_win32.hpp>
)

if(TT_BUILD_TESTS)
    target_sources(ttauri_tests PRIVATE
        audio_sample_unpacker_tests.cpp
        audio_sample_packer_tests.cpp
    )
endif()

