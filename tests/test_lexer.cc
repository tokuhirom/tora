#include "nanotap.h"
#include "../src/lexer.gen.h"

using namespace tora;

int main() {
    ok(true);
    std::stringstream *ss = new std::stringstream(std::string("qw();"));
    Scanner scanner(ss);
    Node *yylval;
    is(scanner.scan(&yylval), QW_START);
    is(scanner.scan(&yylval), QW_END);
    is(scanner.scan(&yylval), SEMICOLON);
    is(scanner.scan(&yylval), 0);
    done_testing();
}

