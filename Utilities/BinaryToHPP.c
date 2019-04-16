
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int usage(char *program, char *str)
{
    fprintf(stderr, "Argument Error: %s\n\n", str);

    fprintf(stderr, "Usage: %s <binary-file> <output-hpp-file>", program);

    return 2;
}

void get_cooked_filename(char *dst, char *src)
{
    char *last_slash_location = strrchr(src, '/');

    if (last_slash_location) {
        strcpy(dst, last_slash_location + 1);
    } else {
        strcpy(dst, src);
    }

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

    char *input_filename = argv[1];
    char *output_filename = argv[2];

    FILE *input_file = fopen(input_filename, "rb");
    FILE *output_file = fopen(output_filename, "wb");

    long input_file_length = get_file_length(input_file);

    char name[256];
    get_cooked_filename(name, input_filename);

    fprintf(output_file, "#pragma once\n\n");
    fprintf(output_file, "#include <cstdint>\n\n");
    fprintf(output_file, "namespace BinaryAssets {\n\n");
    fprintf(output_file, "extern const uint8_t %s[%lli];\n", name, (long long int)input_file_length);
    fprintf(output_file, "extern const uint32_t *u32%s;\n\n", name);
    fprintf(output_file, "#ifdef BINARY_ASSETS_%s_IMPL\n", name);
    fprintf(output_file, "alignas(4) const uint8_t %s[%lli] = {\n", name, (long long int)input_file_length);

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

        fprintf(stderr, "buffer_size %lli\n", (long long int)buffer_size);
    } while (buffer_size == sizeof(buffer));

    fprintf(output_file, "};\n");
    fprintf(output_file, "const uint32_t *u32%s = reinterpret_cast<const uint32_t *>(%s);\n", name, name);
    fprintf(output_file, "#endif\n");
    fprintf(output_file, "}\n");

    fclose(input_file);
    fclose(output_file);
}
