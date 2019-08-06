// Copyright 2019 Pokitec
// All rights reserved.

#include "parser.hpp"
#include "ASTObject.hpp"
#include "ParseContext.hpp"
#include "exceptions.hpp"
#include "TTauri/logging.hpp"
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


ASTObject *parseConfigFile(boost::filesystem::path const &path)
{
    yyscan_t scanner;
    FILE *file;
    string path_string = path.string();
    ParseContext context(path);

    if ((file = fopen(path_string.data(), "rb")) == nullptr) {
        BOOST_THROW_EXCEPTION(FileError("Could not open file")
            << boost::errinfo_file_name(path.string())
            << boost::errinfo_errno(errno)
        );
    }

    if (TTauriConfig_yylex_init(&scanner) != 0) {
        LOG_FATAL("Failed to allocate memory using TTauriConfig_yylex_init()");
    }

    TTauriConfig_yyset_in(file, scanner);

    auto r = TTauriConfig_yyparse(scanner, &context);

    TTauriConfig_yylex_destroy(scanner);
    if (fclose(file) != 0) {
        BOOST_THROW_EXCEPTION(FileError("Could not close file")
            << boost::errinfo_file_name(path.string())
            << boost::errinfo_errno(errno)
        );
    }

    if (r != 0) {
        BOOST_THROW_EXCEPTION(ParseError(context.errorMessage)
            << errinfo_location(context.errorLocation)
        );
    }

    return context.object;
}


}
