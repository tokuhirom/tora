#include "nanotap.h"
#include "../src/lexer.gen.h"

using namespace tora;

int main() {
    {
        std::stringstream *ss = new std::stringstream(std::string("qw();"));
        Scanner scanner(ss);
        Node *yylval;
        is(scanner.scan(&yylval), QW_START);
        is(scanner.scan(&yylval), QW_END);
        is(scanner.scan(&yylval), SEMICOLON);
        is(scanner.scan(&yylval), 0);
    }
    {
        std::stringstream *ss = new std::stringstream(std::string("qw(1 23 hoge);"));
        Scanner scanner(ss);
        Node *yylval;
        is(scanner.scan(&yylval), QW_START);
        is(scanner.scan(&yylval), QW_WORD);
        is(scanner.scan(&yylval), QW_WORD);
        is(scanner.scan(&yylval), QW_WORD);
        is(scanner.scan(&yylval), QW_END);
        is(scanner.scan(&yylval), SEMICOLON);
        is(scanner.scan(&yylval), 0);
    }
    {
        std::stringstream *ss = new std::stringstream(std::string("qw[];"));
        Scanner scanner(ss);
        Node *yylval;
        is(scanner.scan(&yylval), QW_START);
        is(scanner.scan(&yylval), QW_END);
        is(scanner.scan(&yylval), SEMICOLON);
        is(scanner.scan(&yylval), 0);
    }
    {
        std::stringstream *ss = new std::stringstream(std::string("qw!!;"));
        Scanner scanner(ss);
        Node *yylval;
        is(scanner.scan(&yylval), QW_START);
        is(scanner.scan(&yylval), QW_END);
        is(scanner.scan(&yylval), SEMICOLON);
        is(scanner.scan(&yylval), 0);
    }
    {
        std::stringstream *ss = new std::stringstream(std::string("qw{};"));
        Scanner scanner(ss);
        Node *yylval;
        is(scanner.scan(&yylval), QW_START);
        is(scanner.scan(&yylval), QW_END);
        is(scanner.scan(&yylval), SEMICOLON);
        is(scanner.scan(&yylval), 0);
    }
    {
        std::stringstream *ss = new std::stringstream(std::string("4649;"));
        Scanner scanner(ss);
        Node *yylval;
        is(scanner.scan(&yylval), INT_LITERAL);
        is(scanner.scan(&yylval), SEMICOLON);
        is(scanner.scan(&yylval), 0);
    }
    {
        std::stringstream *ss = new std::stringstream(std::string("3.14;"));
        Scanner scanner(ss);
        Node *yylval;
        is(scanner.scan(&yylval), DOUBLE_LITERAL);
        is(yylval->upcast<DoubleNode>()->double_value, 3.14);
        is(scanner.scan(&yylval), SEMICOLON);
        is(scanner.scan(&yylval), 0);
    }
    done_testing();
}

