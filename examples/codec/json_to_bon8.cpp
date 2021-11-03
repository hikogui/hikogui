// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/crt.hpp"
#include "ttauri/URL.hpp"
#include "ttauri/file.hpp"
#include "ttauri/codec/JSON.hpp"
#include "ttauri/codec/BON8.hpp"
#include <string>
#include <format>
#include <ostream>

using namespace tt;

int usage()
{
    std::cerr << "Usage:\n";
    std::cerr << "    json_to_bon8 <json input filename> <bon8 output filename>\n" << std::endl;
    return 2;
}

int tt_main(int argc, char* argv[])
{
    if (argc != 3) {
        return usage();
    }
    auto json_filename = URL(argv[1]);
    auto bon8_filename = URL(argv[2]);

    auto json_view = json_filename.loadView();
    auto json_data = json_view->string_view();
    auto data = parse_JSON(json_data);

    auto bon8_file = file(bon8_filename, access_mode::truncate_or_create_for_write);
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
