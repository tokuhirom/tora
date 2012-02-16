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
#include "disasm.h"

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
    tora::Parser parser;
    do {
        token_number = scanner->scan(&yylval);
        parser.set_lineno(scanner->lineno());
        parser.parse(token_number, yylval);
    } while (token_number != 0);

    if (parser.is_failure()) {
        return 1;
    }

    if (compile_only) {
        printf("Syntax OK\n");
        return 0;
    }

    assert(parser.root_node());
    if (dump_ast) {
        parser.root_node()->dump();
    }

    // compile
    tora::Compiler compiler;
    compiler.init_globals();
    compiler.compile(parser.root_node());
    if (compiler.error) {
        fprintf(stderr, "Compilation failed\n");
        exit(1);
    }

    // run it
    tora::VM vm(compiler.ops);
    vm.init_globals(argc-optind, argv+optind);
    if (dump_ops) {
        Disasm::disasm(compiler.ops);
    }
    vm.execute();
}

