
target_sources(ttauri PRIVATE
    base64.cpp
    base64.hpp
    base16.hpp
    gzip.cpp
    gzip.hpp
    inflate.cpp
    inflate.hpp
    JSON.cpp
    JSON.hpp
    png.cpp
    png.hpp
    SHA2.hpp
    zlib.cpp
    zlib.hpp
)

target_sources(ttauri_tests PRIVATE
    JSON_tests.cpp
    gzip_tests.cpp
    base64_tests.cpp
    base16_tests.cpp
    SHA2_tests.cpp
)
