
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int usage(char *program, char *str)
{
    fprintf(stderr, "Argument Error: %s\n\n", str);

    fprintf(stderr, "Usage: %s <binary-file> <output-hpp-file>", program);

    return 2;
}

void get_filename(char *dst, char *src)
{
    char *last_slash_location = strrchr(src, '/');

    if (last_slash_location) {
        strcpy(dst, last_slash_location + 1);
    } else {
        strcpy(dst, src);
    }
}

void get_c_identifier(char *dst, char *src)
{
    strcpy(dst, src);
    size_t dst_length = strlen(dst);
    for (size_t i = 0; i < dst_length; i++) {
        char c = dst[i];
        if (!(
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9')
        )) {
            dst[i] = '_';
        }
    }
}

long get_file_length(FILE *file)
{
    long original_location = ftell(file);
    fseek(file, 0, SEEK_END);
    long r = ftell(file);
    fseek(file, original_location, SEEK_SET);
    return r;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        return usage(argv[0], "Expected two arguments.");
    }

    char *input_path = argv[1];
    char *output_path = argv[2];

    FILE *input_file = fopen(input_path, "rb");
    if (input_file == NULL) {
        fprintf(stderr, "Could not open input file '%s'.\n", input_path);
        return 1;
    }

    FILE *output_file = fopen(output_path, "wb");
    if (output_file == NULL) {
        fprintf(stderr, "Could not open output file '%s'.\n", output_path);
        return 1;
    }

    long input_file_length = get_file_length(input_file);

    char input_filename[256];
    get_filename(input_filename, input_path);
    char name[256];
    get_c_identifier(name, input_filename);

    fprintf(output_file, "#pragma once\n\n");

    fprintf(output_file, "#include <gsl/gsl>\n");
    fprintf(output_file, "#include <cstdint>\n\n");

    fprintf(output_file, "alignas(8) static const uint8_t %s_data[%llu] = {\n", name, (long long unsigned)input_file_length);

    uint8_t buffer[4096];
    size_t buffer_size;
    do {
        buffer_size = fread(buffer, sizeof(uint8_t), sizeof(buffer), input_file);
        if (buffer_size != sizeof(buffer) && ferror(input_file)) {
            fprintf(stderr, "Read error.\n");
            return 1;
		}

        for (size_t i = 0; i < buffer_size; i++) {
            fprintf(output_file, "0x%02x,", buffer[i]);
            if (i % 16 == 15) {
                fprintf(output_file, "\n");
            }
        }
    } while (buffer_size == sizeof(buffer));

    fprintf(output_file, "};\n\n");

    fprintf(output_file, "static const gsl::span<std::byte const> %s_bytes = {reinterpret_cast<std::byte const *>(%s_data), sizeof(%s_data)};\n",
        name, name, name);
    fprintf(output_file, "static const char *%s_filename = \"%s\";\n", name, input_filename);

    fclose(input_file);
    fclose(output_file);
}
