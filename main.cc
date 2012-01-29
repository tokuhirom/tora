#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <cassert>

#include "tora.h"
#include "vm.h"
#include "compiler.h"

extern TNode *root_node;

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
    bool compile_only = false;
    while ((opt = getopt(argc, argv, "vdc")) != -1) {
        switch (opt) {
        case 'v':
            printf("tora version %s\n", TORA_VERSION_STR);
            exit(EXIT_SUCCESS);
        case 'd':
            dump_ops = true;
            break;
        case 'c':
            compile_only = true;
            break;
        default:
            fprintf(stderr, "Usage: %s [-v] [srcfile]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) { // source code
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
        VM vm;
        tora::Compiler compiler(&vm);
        compiler.compile(root_node);
        if (dump_ops) {
            vm.dump_ops();
        }
        vm.execute();
    }
}
