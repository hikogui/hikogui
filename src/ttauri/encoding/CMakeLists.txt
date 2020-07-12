
target_sources(ttauri PRIVATE
    #base64.cpp
    #base64.hpp
    gzip.cpp
    gzip.hpp
    inflate.cpp
    inflate.hpp
    JSON.cpp
    JSON.hpp
    png.cpp
    png.hpp
    #sha512.cpp
    #sha512.hpp
    zlib.cpp
    zlib.hpp
)

target_sources(ttauri_tests PRIVATE
    JSON_tests.cpp
    gzip_tests.cpp
)
