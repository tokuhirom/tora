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
#include "lexer.gen.cc"

using namespace tora;

extern tora::Node *root_node;

Scanner *scanner;
int yylex() {
    int n = scanner->scan();
    // printf("TOKEN: %d\n", n);
    return n;
}

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
    char *code = NULL;
    while ((opt = getopt(argc, argv, "vdtce:")) != -1) {
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
        default:
            fprintf(stderr, "Usage: %s [-v] [srcfile]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    std::ifstream *ifs;
    if (code) {
        scanner = new Scanner(new std::stringstream(std::string(code)));
    } else if (optind < argc) { // source code
        ifs = new std::ifstream(argv[optind], std::ios::in);
        if (!ifs) { perror(argv[optind]); exit(EXIT_FAILURE); }
        scanner = new Scanner(ifs);
    } else {
        scanner = new Scanner(&std::cin);
    }

    root_node = NULL;
    if (yyparse()) {
        fprintf(stderr, "Syntax Error!! at line %d\n", scanner->lineno());
        exit(1);
    } else {
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
    }
}
