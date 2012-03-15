#include "nanotap.h"
#include "../src/lexer.h"
#include "../src/nodes.gen.h"
#include "../src/parser.h"
#include <boost/scoped_ptr.hpp>
#include <stdarg.h>

using namespace tora;

#define TOKEN_IS(x) is(scanner.scan(&yylval), x)
#define LVAL_STR_IS(x) is(yylval->upcast<StrNode>()->str_value, std::string(x));
#define BEGIN(x) printf("# line %d\n", __LINE__); std::stringstream ss(std::string((const char*)(x))); Scanner scanner(&ss, "<eval>"); Node *yylval = NULL;

int main() {
    {
        BEGIN("qw();");
        TOKEN_IS(QW_START);
        TOKEN_IS(QW_END);
        TOKEN_IS(SEMICOLON);
        TOKEN_IS(0);
    }
    {
        BEGIN("qw(1 23 hoge);");
        TOKEN_IS( QW_START );
        TOKEN_IS( QW_WORD );
        TOKEN_IS( QW_WORD );
        TOKEN_IS( QW_WORD );
        TOKEN_IS( QW_END );
        TOKEN_IS( SEMICOLON );
        TOKEN_IS( 0 );
    }
    {
        BEGIN("qw[];");
        is(scanner.scan(&yylval), QW_START);
        is(scanner.scan(&yylval), QW_END);
        is(scanner.scan(&yylval), SEMICOLON);
        is(scanner.scan(&yylval), 0);
    }
    {
        BEGIN("qw!!;");
        is(scanner.scan(&yylval), QW_START);
        is(scanner.scan(&yylval), QW_END);
        is(scanner.scan(&yylval), SEMICOLON);
        is(scanner.scan(&yylval), 0);
    }
    {
        BEGIN("qw{};");
        is(scanner.scan(&yylval), QW_START);
        is(scanner.scan(&yylval), QW_END);
        is(scanner.scan(&yylval), SEMICOLON);
        is(scanner.scan(&yylval), 0);
    }
    {
        BEGIN("4649;");
        is(scanner.scan(&yylval), INT_LITERAL);
        is(scanner.scan(&yylval), SEMICOLON);
        is(scanner.scan(&yylval), 0);
    }
    {
        BEGIN("3.14;");
        is(scanner.scan(&yylval), DOUBLE_LITERAL);
        is(yylval->upcast<DoubleNode>()->double_value, 3.14);
        is(scanner.scan(&yylval), SEMICOLON);
        is(scanner.scan(&yylval), 0);
    }
    {
        BEGIN("'ab\\'c';");
        is(scanner.scan(&yylval), STRING_LITERAL);
        is(yylval->upcast<StrNode>()->str_value, std::string("ab'c"));
        is(scanner.scan(&yylval), SEMICOLON);
        is(scanner.scan(&yylval), 0);
    }
    {
        BEGIN("q{ab'c};");
        TOKEN_IS(STRING_LITERAL);
        LVAL_STR_IS("ab'c");
        TOKEN_IS(SEMICOLON);
        TOKEN_IS(0);
    }
    {
        BEGIN("q(ab'c);");
        TOKEN_IS(STRING_LITERAL);
        LVAL_STR_IS("ab'c");
        TOKEN_IS(SEMICOLON);
        TOKEN_IS(0);
    }
    {
        BEGIN("q!ab'c!;");
        TOKEN_IS(STRING_LITERAL);
        LVAL_STR_IS("ab'c");
        TOKEN_IS(SEMICOLON);
        TOKEN_IS(0);
    }
    {
        BEGIN("q[ab'c];");
        TOKEN_IS(STRING_LITERAL);
        LVAL_STR_IS("ab'c");
        TOKEN_IS(SEMICOLON);
        TOKEN_IS(0);
    }
    {
        BEGIN("\"abc\";");
        TOKEN_IS(STRING_LITERAL);
        LVAL_STR_IS("abc");
        TOKEN_IS(SEMICOLON);
        TOKEN_IS(0);
    }
    {
        const char *xx[] = {
            "\"ab\'\\\"c\";",
            "qq{ab'\"c};",
            "qq(ab'\"c);",
            "qq[ab'\"c];",
            "qq!ab'\"c!;",
            NULL
        };
        for (int i=0; xx[i]; i++) {
            BEGIN(xx[i]);
            TOKEN_IS(STRING_LITERAL);
            LVAL_STR_IS("ab'\"c");
            TOKEN_IS(SEMICOLON);
            TOKEN_IS(0);
        }
    }
    {
        BEGIN("\'{}\';");
        TOKEN_IS(STRING_LITERAL);
        LVAL_STR_IS("{}");
        TOKEN_IS(SEMICOLON);
        TOKEN_IS(0);
    }
    {
        BEGIN("\"{}\";");
        TOKEN_IS(STRING_LITERAL);
        LVAL_STR_IS("{}");
        TOKEN_IS(SEMICOLON);
        TOKEN_IS(0);
    }
    done_testing();
}

