# Copyright Take Vos 2021-2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

target_sources(hikogui PRIVATE
    audio_block.hpp
    audio_channel.hpp
    audio_device.cpp
    audio_device.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/audio_device_asio.hpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/audio_device_win32.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/audio_device_win32.hpp>
    audio_device_delegate.hpp
    audio_stream_config.hpp
    audio_stream_format.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/audio_stream_format_win32.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/audio_stream_format_win32.hpp>
    audio_system.cpp
    audio_system.hpp
    audio_system_aggregate.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/audio_system_asio.hpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/audio_system_win32.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/audio_system_win32.hpp>
    audio_sample_format.cpp
    audio_sample_format.hpp
    audio_sample_unpacker.cpp
    audio_sample_unpacker.hpp
    audio_sample_packer.cpp
    audio_sample_packer.hpp
    pcm_format.hpp
    speaker_mapping.cpp
    speaker_mapping.hpp
    surround_mode.hpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/speaker_mapping_win32.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/speaker_mapping_win32.hpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/win32_device_interface.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/win32_device_interface.hpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/win32_wave_device.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/win32_wave_device.hpp>
)

if(BUILD_TESTING)
    target_sources(hikogui_tests PRIVATE
        audio_sample_unpacker_tests.cpp
        audio_sample_packer_tests.cpp
    )
endif()

