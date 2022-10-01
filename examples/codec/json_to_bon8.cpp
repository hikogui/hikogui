// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/crt.hpp"
#include "hikogui/file/file_view.hpp"
#include "hikogui/codec/JSON.hpp"
#include "hikogui/codec/BON8.hpp"
#include <string>
#include <format>
#include <ostream>
#include <filesystem>

int usage()
{
    std::cerr << "Usage:\n";
    std::cerr << "    json_to_bon8 <json input filename> <bon8 output filename>\n" << std::endl;
    return 2;
}

int hi_main(int argc, char* argv[])
{
    if (argc != 3) {
        return usage();
    }
    auto json_filename = std::filesystem::path(argv[1]);
    auto bon8_filename = std::filesystem::path(argv[2]);

    auto json_view = hi::file_view(json_filename);
    auto json_data = as_string_view(json_view);
    auto data = hi::parse_JSON(json_data);

    auto bon8_file = hi::file(bon8_filename, hi::access_mode::truncate_or_create_for_write);
    auto bon8_data = encode_BON8(data);
    bon8_file.write(bon8_data);
    bon8_file.close();

    auto data_read_back = decode_BON8(bon8_data);

    if (data == data_read_back) {
        std::cout << "Data was read back correctly" << std::endl;
    } else {
        std::cout << "Error BON8 encode -> decode failure" << std::endl;
    }

    std::cout << std::format("json {}, bon8 {}, compression {:.1f}", size(json_data), size(bon8_data), (static_cast<double>(size(bon8_data)) / static_cast<double>(size(json_data))) * 100.0) << std::endl;

    return 0;
}
