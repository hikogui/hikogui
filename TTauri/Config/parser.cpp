// Copyright 2019 Pokitec
// All rights reserved.

#include "parser.hpp"
#include "ASTObject.hpp"
#include "ParseContext.hpp"
#include "TTauri/exceptions.hpp"
#include "TTauri/logger.hpp"
#include <cstdio>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

typedef void *yyscan_t;

int TTauriConfig_yylex_init(yyscan_t *scanner);
int TTauriConfig_yylex_destroy(yyscan_t scanner);
void TTauriConfig_yyset_in(FILE *file, yyscan_t scanner);
int TTauriConfig_yyparse(yyscan_t scanner, TTauri::Config::ParseContext *context);

namespace TTauri::Config {

using namespace std;


ASTObject *parseConfigFile(URL const &path)
{
    yyscan_t scanner;
    FILE *file;
    string path_string = path.path_string();
    ParseContext context(path);

    if ((file = fopen(path_string.data(), "rb")) == nullptr) {
        TTAURI_THROW(io_error("Could not open file")
            << error_info<"url"_tag>(path)
            << error_info<"errno"_tag>(errno)
        );
    }

    if (TTauriConfig_yylex_init(&scanner) != 0) {
        LOG_FATAL("Failed to allocate memory using TTauriConfig_yylex_init()");
    }

    TTauriConfig_yyset_in(file, scanner);

    auto r = TTauriConfig_yyparse(scanner, &context);

    TTauriConfig_yylex_destroy(scanner);
    if (fclose(file) != 0) {
        TTAURI_THROW(io_error("Could not close file")
            << error_info<"url"_tag>(path)
            << error_info<"errno"_tag>(errno)
        );
    }

    if (r != 0) {
        TTAURI_THROW(parse_error(context.errorMessage)
            << error_info<"location"_tag>(context.errorLocation)
        );
    }

    return context.object;
}


}
