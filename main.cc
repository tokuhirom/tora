#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <cassert>

#include "tora.h"
#include "vm.h"
#include "compiler.h"

using namespace tora;

extern Node *root_node;

int main(int argc, char **argv) {
    extern int yyparse(void);
    extern FILE *yyin;
    yyin = stdin;
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

    if (code) {
        FILE *fh = tmpfile();
        fprintf(fh, "%s;", code);
        rewind(fh);
        yyin = fh;
    } else if (optind < argc) { // source code
        FILE *fh = fopen(argv[optind], "rb");
        if (!fh) { perror(argv[optind]); exit(EXIT_FAILURE); }
        yyin = fh;
    } else {
        yyin = stdin;
    }

    root_node = NULL;
    if (yyparse()) {
        extern int tora_line_number;
        fprintf(stderr, "Error! at line %d\n", tora_line_number);
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

        tora::VM vm;
        tora::Compiler compiler(&vm);
        compiler.compile(root_node);
        if (compiler.error) {
            fprintf(stderr, "Compilation failed\n");
            exit(1);
        }
        if (dump_ops) {
            vm.dump_ops();
        }
        vm.execute();
    }
}
