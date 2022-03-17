// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <format>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <string_view>
#include <string>

template<typename... Args>
void print(std::string_view fmt, Args const &... args) noexcept
{
    std::cerr << std::vformat(fmt, std::make_format_args(args...)) << std::endl;
}

void usage(std::string_view program, std::string_view str)
{
    print("Argument Error: {}\n", str);
    print("Usage: {} <binary-file> <output-hpp-file>", program);
    exit(2);
}

[[nodiscard]] std::size_t file_size(std::istream& stream) noexcept
{
    stream.seekg(0, std::ios_base::end);
    auto r = stream.tellg();
    stream.seekg(0);
    return static_cast<std::size_t>(r);
}

[[nodiscard]] std::string read_all(std::istream& stream) noexcept
{
    auto r = std::string(file_size(stream), '\0');
    stream.read(r.data(), r.size());
    return r;
}

template<typename... Args>
void write(std::ostream& stream, std::string_view fmt, Args const &... args) noexcept
{
    stream << std::vformat(fmt, std::make_format_args(args...));
}

void write_bytes_as_text(std::ostream& stream, std::string_view bytes) noexcept
{
    std::size_t i = 0;
    for (auto const c : bytes) {
        write(stream, "0x{:02x},", static_cast<unsigned short>(static_cast<unsigned char>(c)));

        if (++i % 16 == 0) {
            write(stream, "\n");
        }
    }
}

[[nodiscard]] std::string read_file(std::filesystem::path const& path) noexcept
{
    std::ifstream input_file;
    try {
        input_file = std::ifstream(path, std::ios_base::binary);
    }
    catch (std::exception const& e) {
        print("Could not open input file '{}'. '{}'", path.generic_string(), e.what());
        std::exit(1);
    }
    auto const r = read_all(input_file);
    return r;
}

[[nodiscard]] std::ofstream open_output(std::filesystem::path const& path) noexcept
{
    std::ofstream r;
    try {
        r = std::ofstream(path, std::ios_base::trunc);
    }
    catch (std::exception const& e) {
        print("Could not open output file '{}'. '{}'", path.generic_string(), e.what());
        std::exit(1);
    }
    return r;
}

[[nodiscard]] std::string make_identifier(std::string const& str) noexcept
{
    auto r = str;
    for (auto& c : r) {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')) {
            c = '_';
        }
    }
    return r;
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        usage(argv[0], "Expected two arguments.");
    }

    auto const input_path = std::filesystem::path(argv[1]);
    auto const input_data = read_file(input_path); // lgtm[cpp/path-injection]
    auto const input_filename = input_path.filename();
    auto const identifier = make_identifier(input_filename.generic_string());

    auto const output_path = std::filesystem::path(argv[2]);
    auto os = open_output(output_path); // lgtm[cpp/path-injection]

    write(os, "#include \"ttauri/static_resource_list.hpp\"\n");
    write(os, "#include \"ttauri/architecture.hpp\"\n");
    write(os, "#include <span>\n");
    write(os, "#include <cstddef>\n");
    write(os, "#include <cstdint>\n\n");

    write(os, "extern const uint8_t {}_srd[{}];\n", identifier, input_data.size());
    write(os, "extern tt::static_resource_item {}_sri;\n", identifier);
    write(os, "extern \"C\" {{\n");
    write(os, "extern tt::static_resource_item const *{}_srip;\n\n", identifier);
    write(os, "}}\n");

    write(os, "alignas(8) const uint8_t {}_srd[{}] = {{\n", identifier, input_data.size());
    write_bytes_as_text(os, input_data);
    write(os, "}};\n\n");

    write(os, "tt::static_resource_item {}_sri = {{\n", identifier);
    write(os, "    nullptr,\n");
    write(os, "    \"{}\",\n", input_filename.generic_string());
    write(os, "    {{reinterpret_cast<std::byte const *>({}_srd), {}}}\n", identifier, input_data.size());
    write(os, "}};\n\n");

    write(os, "extern \"C\" {{\n");
    write(os, "tt::static_resource_item const *{}_srip = tt::static_resource_item::add(&{}_sri);\n", identifier, identifier);
    write(os, "}}\n\n");
}
