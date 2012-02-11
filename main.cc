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
#include "lexer.gen.cc"
#include "parser.cc"

using namespace tora;

extern tora::Node *root_node;

int main(int argc, char **argv) {
    extern int yyparse(void);
#ifdef YYDEBUG
    extern int yydebug;
    yydebug = 1;
#endif

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
        ParseTrace(stderr, (char*)"[Parser] >> ");
    }

    root_node = NULL;

    YYSTYPE yylval;
    int token_number;
    void *parser = ParseAlloc(malloc);
    do {
        token_number = scanner->scan(&yylval);
        Parse(parser, token_number, yylval);
    } while (token_number != 0);
    ParseFree(parser, free);

    if (compile_only) {
        printf("Syntax OK\n");
        return 0;
    }

    assert(root_node);
    if (dump_ast) {
        root_node->dump();
    }

    tora::Compiler compiler;
    compiler.compile(root_node);
    if (compiler.error) {
        fprintf(stderr, "Compilation failed\n");
        exit(1);
    }
    tora::VM vm(compiler.ops);
    // TODO: use delete
    // delete root_node; // AST is not needed after compiling.
    if (dump_ops) {
        vm.dump_ops();
    }
    vm.execute();

    /*
    fprintf(stderr, "Syntax Error!! at line %d\n", scanner->lineno());
    exit(1);
    */
}
