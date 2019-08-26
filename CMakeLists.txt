
cmake_minimum_required(VERSION 3.0)

if (MSVC)
    add_compile_options(/wd4068 /nologo)
    add_link_options(/nologo)
	add_definitions(-DWIN32 -D_WINDOWS -DUNICODE -D_UNICODE -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)

    if (NOT (${CMAKE_BUILD_TYPE} MATCHES Debug))
        # Whole program optimization.
        #add_compile_options(/GL)
        #add_link_options(/LTCG)
    endif()

endif ()

add_subdirectory(Utilities)
add_subdirectory(ThirdPartyLibraries)
add_subdirectory(TTauri)
add_subdirectory(TTauriDevelopmentApplication)
 
enable_testing()

add_test(NAME TTauriTests COMMAND TTauri/TTauriTests --gtest_break_on_failure --gtest_catch_exceptions=0)
 
 
