#ifdef _WIN32
// http://gcc.gnu.org/bugzilla/show_bug.cgi?id=40278
#undef __STRICT_ANSI__
#endif

#include <stdio.h>
#include <wchar.h>
#include <iostream>
#include <istream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <cassert>
#include <sstream>
#include <memory>

#include "tora.h"
#include "vm.h"
#include "compiler.h"
#include "node.h"
#include "lexer.h"
#include "parser.class.h"
#include "token.gen.h"
#include "disasm.h"
#include "config.h"
#include "value/file_package.h"
#include "value/exception.h"

using namespace tora;

static void show_configuration() {
  printf("Tora %s\n", TORA_VERSION_STR);
  printf("Build configuration: %s\n", TORA_CCFLAGS);
  printf("Install prefix: %s\n", TORA_PREFIX);
}

static void run_repl(const std::vector<std::string>& args,
                     const std::vector<std::string>& libs) {
  bool dump_ops = false;

  SharedPtr<Scanner> scanner = new Scanner(&std::cin, "<stdin>");
  SharedPtr<SymbolTable> symbol_table = new SymbolTable();
  tora::Compiler compiler(symbol_table, "<repl>");
  compiler.init_globals();
  // compiler.compile(parser.root_node());

  tora::VM vm(compiler.ops, symbol_table, dump_ops);
  vm.register_standard_methods();
  vm.init_globals(args);
  for (auto iter = libs.begin(); iter != libs.end(); iter++) {
    vm.add_library_path(*iter);
  }
  vm.add_library_path(std::string(TORA_PREFIX) + "/lib/tora-" +
                      TORA_VERSION_STR);

  std::string buf;
  while (!std::cin.eof()) {
    try {
      std::cout << ">> ";
      std::getline(std::cin, buf, '\n');
      std::stringstream iss(buf + ";");
      SharedPtr<tora::Value> ret = vm.eval(&iss, "<repl>");
      vm.dump_value(ret);
    }
    catch (ExceptionValue* e) {
      std::cerr << exception_message(e) << std::endl;
    };
  }
}

namespace tora {
class CommandLineParser {
  int optind_;

 public:
  bool help;
  bool vernum;
  std::string eval;
  std::vector<std::string> libs;

  int optind() const { return optind_; }
  void parse(int argc, char** argv) {}
};
};

int main(int argc, char** argv) {
  bool dump_ops = false;
  bool dump_ast = false;
  bool compile_only = false;
  bool parse_trace = false;
  bool exec_trace = false;
  std::vector<std::string> libs;
  std::vector<std::string> args;
  std::string code;

  int optind = 1;

  while (optind < argc) {
    std::string arg(argv[optind]);
    if (arg == "--") {
      break;
    } else if (arg == "--help") {
      printf("Usage");
    } else if (arg == "--vernum") {
      std::cout << TORA_VERSION_STR << std::endl;
      exit(EXIT_SUCCESS);
    } else if (arg == "--version-full") {
      show_configuration();
      exit(EXIT_SUCCESS);
    } else if (arg == "--Dtree") {
      dump_ast = true;
    } else if (arg == "--Dops" || arg == "-t") {
      dump_ops = true;
    } else if (arg == "-e" || arg == "--eval") {
      optind++;
      if (optind < argc) {
        code = argv[optind];
      }
    } else if (arg == "-I" || arg == "--include") {
      optind++;
      if (optind < argc) {
        libs.push_back(argv[optind]);
      }
    } else if (arg == "--compile-only" || arg == "-c") {
      compile_only = true;
    } else if (arg == "--parse-trace" || arg == "-y") {
      parse_trace = true;
    } else if (arg == "--exec-trace" || arg == "-q") {
      exec_trace = true;
    } else if (arg[0] != '-') {
      break;
    }
    optind++;
  }
  while (optind < argc) {
    args.push_back(argv[optind]);
    optind++;
  }

  /*
  try {
      po::options_description desc("Allowed options");
      desc.add_options()
          ("help", "produce help message")
          ("vernum", "show version number")
          ("version,v", "show version information")
          ("version-full,V", "show full version information")
          ("Dtree,t", "dump abstract syntax tree")
          ("Dops,d", "dump ops")
          ("eval,e", po::value<std::string>(&code), "evaluate one liner")
          ("include,I", po::value< std::vector<std::string> >(&libs), "library
  include path")
          ("compile-only,c", "only compile")
          ("parse-trace,y", "parse trace")
          ("exec-trace,q", "exec trace")
      ;
      po::options_description positional_desc;
      positional_desc.add_options()
          ("positional", po::value<std::vector<std::string> >(&args))
      ;
      po::options_description all_desc;
      all_desc.add(desc).add(positional_desc);
      po::positional_options_description posops;
      posops.add("positional", -1);

      po::variables_map varmap;
      po::store(po::command_line_parser(argc,
  argv).options(all_desc).positional(posops).run(), varmap);
      po::notify(varmap);

      if (varmap.count("help")) {
          std::cerr << desc << std::endl;
          return EXIT_FAILURE;
      }
      if (varmap.count("vernum")) {
          std::cout << TORA_VERSION_STR << std::endl;
          return EXIT_FAILURE;
      }
      if (varmap.count("version")) {
          printf("tora version %s\n", TORA_VERSION_STR);
          exit(EXIT_SUCCESS);
      }
      if (varmap.count("version-full")) {
          show_configuration();
          exit(EXIT_SUCCESS);
      }
      if (varmap.count("Dtree")) {
          dump_ast = true;
      }
      if (varmap.count("Dops")) {
          dump_ops = true;
      }
      if (varmap.count("compile-only")) {
          compile_only = true;
      }
      if (varmap.count("parse-trace")) {
          parse_trace = true;
      }
      if (varmap.count("exec-trace")) {
          exec_trace = true;
      }
  } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
  }
  */

  std::unique_ptr<std::ifstream> ifs;
  std::unique_ptr<std::stringstream> ss;
  SharedPtr<Scanner> scanner;
  std::string filename;
  if (code.length()) {
    ss.reset(new std::stringstream(std::string(code) + ";"));
    filename = "<eval>";
    scanner = new Scanner(ss.get(), "<eval>");
  } else if (args.size() > 0) {  // source code
    filename = args[0];
    ifs.reset(new std::ifstream(filename, std::ios::in));
    assert(ifs);
    if (!ifs->is_open()) {
      fprintf(stderr, "Cannot open source file '%s': %s\n", filename.c_str(),
              strerror(errno));
      exit(EXIT_FAILURE);
    }
    scanner = new Scanner(ifs.get(), filename);
  } else {
    if (isatty(fileno(stdin)) && isatty(fileno(stdout))) {
      run_repl(args, libs);
      return 0;
    } else {
      filename = "<stdin>";
      scanner = new Scanner(&std::cin, "<stdin>");
    }
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

  Node* yylval = NULL;
  int token_number;
  tora::Parser parser(filename);
  do {
    token_number = scanner->scan(&yylval);
    // printf("TOKEN: %s(%d)\n", token_id2name[token_number], token_number);
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
  tora::Compiler compiler(symbol_table, filename);
  compiler.dump_ops = dump_ops;
  compiler.init_globals();
  compiler.compile(parser.root_node());
  if (compiler.error) {
    fprintf(stderr, "Compilation failed\n");
    exit(1);
  }

  // run it

  tora::VM vm(compiler.ops, symbol_table, dump_ops);
  vm.register_standard_methods();
  vm.init_globals(args);
  for (auto iter = libs.begin(); iter != libs.end(); iter++) {
    vm.add_library_path(*iter);
  }
  vm.add_library_path(std::string(TORA_PREFIX) + "/lib/tora-" +
                      TORA_VERSION_STR);
  if (dump_ops) {
    Disasm::disasm(compiler.ops);
  }
  vm.exec_trace = exec_trace;
  vm.execute();
}
