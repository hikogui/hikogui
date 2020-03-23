set(gtest_force_shared_crt ON)

if (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)    # png comparison is always false.
    add_compile_options(-Wno-tautological-constant-out-of-range-compare)
    if(NOT APPLE)
        # rhea destructor called on non-final that has virtual function.
        add_compile_options(-Wno-delete-non-abstract-non-virtual-dtor)
    endif()
    # rhea /MP argument.
    add_compile_options(-Wno-unused-command-line-argument)
elseif(MSVC)
    # date/tz argument conversion
    add_compile_options(-wd4244)
endif()

add_subdirectory(googletest)
add_subdirectory(GSL)
add_subdirectory(zlib)
add_subdirectory(libpng)
add_subdirectory(VulkanMemoryAllocator)
add_subdirectory(rhea)
add_subdirectory(fmt)
add_subdirectory(date)

