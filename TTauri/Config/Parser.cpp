// Copyright 2019 Pokitec
// All rights reserved.

#include "parser.hpp"
#include "AST.hpp"
#include "ParseContext.hpp"
#include "TTauri/utils.hpp"
#include "TTauri/Logging.hpp"
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


ASTObject *parseFile(const std::filesystem::path &path)
{
    yyscan_t scanner;
    FILE *file;
    string path_string = path.string();
    ASTObject *r;
    ParseContext context;

    if ((file = fopen(path_string.data(), "rb")) == nullptr) {
        BOOST_THROW_EXCEPTION(CanNotOpenFile());
    }

    if (TTauriConfig_yylex_init(&scanner) != 0) {
        BOOST_THROW_EXCEPTION(InternalParserError());
    }

    TTauriConfig_yyset_in(file, scanner);

    if (TTauriConfig_yyparse(scanner, &context) == 0) {
        r = context.object;
    } else {
        LOG_ERROR("%i:%i: %s") % context.errorLocation.firstLine % context.errorLocation.firstColumn % context.errorMessage;
        r = nullptr;
    }

    TTauriConfig_yylex_destroy(scanner);
    if (fclose(file) != 0) {
        BOOST_THROW_EXCEPTION(CanNotCloseFile());
    }
    return r;
}


}
