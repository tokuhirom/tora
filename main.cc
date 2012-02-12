#include <stdio.h>
#include <iostream>
#include <istream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <cassert>
#include <sstream>

#include "tora.h"
#include "vm.h"
#include "compiler.h"
#include "node.h"
#include "lexer.h"
#include "lexer.gen.h"
#include "parser.class.h"

using namespace tora;

int main(int argc, char **argv) {
    char opt;
    bool dump_ops = false;
    bool dump_ast = false;
    bool compile_only = false;
    bool parse_trace = false;
    char *code = NULL;
    while ((opt = getopt(argc, argv, "yvdtce:")) != -1) {
        switch (opt) {
        case 'v':
            printf("tora version %s\n", TORA_VERSION_STR);
            exit(EXIT_SUCCESS);
        case 't':
            dump_ast = true;
            break;
        case 'd':
            dump_ops = true;
            break;
        case 'e':
            code = optarg;
            break;
        case 'c':
            compile_only = true;
            break;
        case 'y':
            parse_trace = true;
            break;
        default:
            fprintf(stderr, "Usage: %s [-v] [srcfile]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    std::ifstream *ifs;
    Scanner *scanner;
    if (code) {
        std::stringstream *s = new std::stringstream(std::string(code) + ";");
        scanner = new Scanner(s);
    } else if (optind < argc) { // source code
        ifs = new std::ifstream(argv[optind], std::ios::in);
        if (!ifs) { perror(argv[optind]); exit(EXIT_FAILURE); }
        scanner = new Scanner(ifs);
    } else {
        scanner = new Scanner(&std::cin);
    }

    if (parse_trace) {
        tora::Parser::ParseTrace(stderr, (char*)"[Parser] >> ");
    }


    Node *yylval;
    int token_number;
    void *parser = tora::Parser::ParseAlloc(malloc);
    ParserState parser_state;
    do {
        token_number = scanner->scan(&yylval);
        parser_state.lineno = scanner->lineno();
        tora::Parser::Parse(parser, token_number, yylval, &parser_state);
    } while (token_number != 0);
    tora::Parser::ParseFree(parser, free);

    if (parser_state.failure) {
        return 1;
    }

    if (compile_only) {
        printf("Syntax OK\n");
        return 0;
    }

    assert(parser_state.root_node);
    if (dump_ast) {
        parser_state.root_node->dump();
    }

    // compile
    tora::Compiler compiler;
    compiler.init_globals();
    compiler.compile(parser_state.root_node);
    if (compiler.error) {
        fprintf(stderr, "Compilation failed\n");
        exit(1);
    }

    // run it
    tora::VM vm(compiler.ops);
    vm.init_globals(argc-optind, argv+optind);
    if (dump_ops) {
        vm.dump_ops();
    }
    vm.execute();
}

