#include <stdio.h>
#include <iostream>
#include <istream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <cassert>
#include <sstream>
#include <memory>

#include <boost/scoped_ptr.hpp>

#include "tora.h"
#include "vm.h"
#include "compiler.h"
#include "node.h"
#include "lexer.h"
#include "parser.class.h"
#include "token.gen.h"
#include "disasm.h"
#include "config.h"
#include "frame.h"

using namespace tora;

void show_configuration() {
    printf("Tora %s\n", TORA_VERSION_STR);
    printf("Build configuration: %s\n", TORA_CCFLAGS);
}

int main(int argc, char **argv) {
    char opt;
    bool dump_ops = false;
    bool dump_ast = false;
    bool compile_only = false;
    bool parse_trace = false;
    bool exec_trace = false;
    char *code = NULL;
    while ((opt = getopt(argc, argv, "Vyvdtcqe:")) != -1) {
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
        case 'V':
            show_configuration();
            exit(EXIT_SUCCESS);
            break;
        case 'q':
            exec_trace = true;
            break;
        default:
            fprintf(stderr, "Usage: %s [-v] [srcfile]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    boost::scoped_ptr<std::ifstream> ifs;
    boost::scoped_ptr<std::stringstream> ss;
    SharedPtr<Scanner> scanner;
    if (code) {
        ss.reset(new std::stringstream(std::string(code) + ";"));
        scanner = new Scanner(ss.get(), "<eval>");
    } else if (optind < argc) { // source code
        ifs.reset(new std::ifstream(argv[optind], std::ios::in));
        assert(ifs);
        if (!ifs->is_open()) {
            perror(argv[optind]);
            exit(EXIT_FAILURE);
        }
        scanner = new Scanner(ifs.get(), argv[optind]);
        optind++;
    } else {
        scanner = new Scanner(&std::cin, "<stdin>");
    }

#ifndef NDEBUG
    if (parse_trace) {
        tora::Parser::ParseTrace(stderr, (char*)"[Parser] >> ");
    }
#else
    if (parse_trace) {
        fprintf(stderr, "Parsing trace is not supported on -DNDEBUG\n");
    }
#endif

    Node *yylval = NULL;
    int token_number;
    tora::Parser parser;
    do {
        token_number = scanner->scan(&yylval);
        // printf("TOKEn: %s(%d)\n", token_id2name[token_number], token_number);
        parser.set_lineno(scanner->lineno());
        parser.parse(token_number, yylval);
    } while (token_number != 0);
    if (scanner->in_heredoc()) {
        fprintf(stderr, "Unexpected EOF in heredoc\n");
        exit(1);
    }

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
    SharedPtr<SymbolTable> symbol_table = new SymbolTable();
    tora::Compiler compiler(symbol_table);
    compiler.dump_ops = dump_ops;
    compiler.init_globals();
    compiler.compile(parser.root_node());
    if (compiler.error) {
        fprintf(stderr, "Compilation failed\n");
        exit(1);
    }

    // run it
    // tora::FunctionFrame::initMemPool();

    tora::VM vm(compiler.ops, symbol_table);
    vm.init_globals(argc-optind, argv+optind);
    vm.register_standard_methods();
    if (dump_ops) {
        Disasm::disasm(compiler.ops);
    }
    vm.exec_trace = exec_trace;
    vm.execute();
}

