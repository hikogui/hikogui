// Copyright 2019 Pokitec
// All rights reserved.

#include "parser.hpp"
#include "AST.hpp"
#include "ParseContext.hpp"
#include "exceptions.hpp"
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
    ParseContext context(path);

    if ((file = fopen(path_string.data(), "rb")) == nullptr) {
        BOOST_THROW_EXCEPTION(CanNotOpenFile()
            << boost::errinfo_file_name(context.errorLocation.file->string())
        );
    }

    if (TTauriConfig_yylex_init(&scanner) != 0) {
        BOOST_THROW_EXCEPTION(InternalParserError());
    }

    TTauriConfig_yyset_in(file, scanner);

    auto r = TTauriConfig_yyparse(scanner, &context);

    TTauriConfig_yylex_destroy(scanner);
    if (fclose(file) != 0) {
        BOOST_THROW_EXCEPTION(CanNotCloseFile()
            << boost::errinfo_file_name(context.errorLocation.file->string())
        );
    }

    if (r != 0) {
        BOOST_THROW_EXCEPTION(ParseError()
            << boost::errinfo_file_name(context.errorLocation.file->string())
            << boost::errinfo_at_line(context.errorLocation.line)
            << errinfo_at_column(context.errorLocation.column)
            << errinfo_message(context.errorMessage)
        );
    }

    return context.object;
}


}
