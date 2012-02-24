#include "vm.h"
#include "value.h"
#include "value/hash.h"
#include "value/code.h"
#include "value/regexp.h"
#include "value/file.h"
#include "lexer.gen.h"
#include "parser.class.h"
#include "compiler.h"
#include "disasm.h"
#include <unistd.h>
#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

using namespace tora;

VM::VM(SharedPtr<OPArray>& ops_, SharedPtr<SymbolTable> &symbol_table_) {
    sp = 0;
    pc = 0;
    symbol_table = symbol_table_;
    ops = ops_;
    this->frame_stack = new std::vector<SharedPtr<LexicalVarsFrame>>();
    this->frame_stack->push_back(new LexicalVarsFrame(0));
    this->global_vars = new std::vector<SharedPtr<Value>>();
    this->package = "main";
    this->package_map = new PackageMap();
}

VM::~VM() {
    delete this->global_vars;
    delete this->frame_stack;

    {
        auto iter = standard.begin();
        for (; iter!=standard.end(); iter++) {
            auto mp = iter->second;
            auto iter2 = mp->begin();
            for (; iter2!=mp->end(); iter2++) {
                delete iter2->second;
            }
            delete mp;
        }
    }

    {
        // std::map<ID, CallbackFunction*> builtin_functions;
        auto iter = builtin_functions.begin();
        for (;iter != builtin_functions.end(); iter++) {
            delete iter->second;
        }
    }
}

void VM::init_globals(int argc, char**argv) {
    // $ARGV
    SharedPtr<ArrayValue> avalue = new ArrayValue();
    for (int i=0; i<argc; i++) {
        avalue->push(new StrValue(argv[i]));
    }
    this->global_vars->push_back(avalue);

    // $ENV
    this->global_vars->push_back(UndefValue::instance());

    // $LIBPATH : Array
    SharedPtr<ArrayValue> libpath = new ArrayValue();
    libpath->push(new StrValue("lib"));
    this->global_vars->push_back(libpath);

    // $REQUIRED : Hash
    this->global_vars->push_back(new HashValue());
}

template <class operationI, class operationD>
void VM::binop(operationI operation_i, operationD operation_d) {
    SharedPtr<Value> v1(stack.pop()); /* rvalue */
    SharedPtr<Value> v2(stack.pop()); /* lvalue */

    if (v2->value_type == VALUE_TYPE_DOUBLE) {
        if (v1->value_type == VALUE_TYPE_DOUBLE) {
            SharedPtr<DoubleValue>v = new DoubleValue(operation_d(v2->upcast<DoubleValue>()->double_value, v1->upcast<DoubleValue>()->double_value));
            stack.push(v);
        } else if (v1->value_type == VALUE_TYPE_INT) {
            SharedPtr<DoubleValue>v = new DoubleValue(operation_d(v2->upcast<DoubleValue>()->double_value, (double)v1->upcast<IntValue>()->int_value));
            stack.push(v);
        }
    } else if (v2->value_type == VALUE_TYPE_INT) {
        SharedPtr<IntValue> v = new IntValue(operation_i(v2->upcast<IntValue>()->int_value, v1->upcast<IntValue>()->int_value));
        stack.push(v);
    } else { 
        SharedPtr<Value> s(v2->to_s());
        fprintf(stderr, "'%s' is not numeric.\n", s->upcast<StrValue>()->str_value.c_str());
        exit(1); /* TODO: die */
    }
}

template <class operationI, class operationD>
void VM::cmpop(operationI operation_i, operationD operation_d) {
    SharedPtr<Value> v1(stack.pop());
    SharedPtr<Value> v2(stack.pop());
 
    switch (v1->value_type) {
    case VALUE_TYPE_INT: {
        SharedPtr<Value> i2(v2->to_i());
        SharedPtr<BoolValue> result = BoolValue::instance(operation_i(v1->upcast<IntValue>()->int_value, i2->upcast<IntValue>()->int_value));
        stack.push(result);
        break;
    }
    default:
        printf("UNKNOWN MATCHING PATTERN:: %s\n", opcode2name[ops->at(pc)->op_type]);
        v1->dump();
        v2->dump();
        abort();
    }
}

void VM::die(const char *format, ...) {
    va_list ap;
    char p[4096+1];
    va_start(ap, format);
    vsnprintf(p, 4096, format, ap);
    va_end(ap);
    std::string s = p;
    SharedPtr<Value> v(new StrValue(s));
    this->die(v);
}

void VM::die(SharedPtr<Value> & exception) {
    while (1) {
        if (frame_stack->size() == 1) {
            if (exception->value_type == VALUE_TYPE_STR) {
                fprintf(stderr, "%s\n", exception->upcast<StrValue>()->str_value.c_str());
            } else if (exception->value_type == VALUE_TYPE_EXCEPTION && exception->upcast<ExceptionValue>()->exception_type == EXCEPTION_TYPE_GENERAL) {
                fprintf(stderr, "%s\n", exception->upcast<ExceptionValue>()->message().c_str());
            } else {
                fprintf(stderr, "died\n");
                exception->dump();
            }
            exit(1);
            break;
        }

        SharedPtr<LexicalVarsFrame> frame = frame_stack->back();
        if (frame->type == FRAME_TYPE_FUNCTION) {
            SharedPtr<FunctionFrame> fframe = frame->upcast<FunctionFrame>();
            pc = fframe->return_address;
            ops = fframe->orig_ops;

            while (stack.size() > frame->top) {
                stack.pop();
            }

            frame_stack->pop_back();
        } else if (frame->type == FRAME_TYPE_TRY) {
            SharedPtr<TryFrame> tframe = frame->upcast<TryFrame>();
            pc = tframe->return_address;

            while (stack.size() > frame->top) {
                stack.pop();
            }
            SharedPtr<TupleValue> t = new TupleValue();
            t->push(UndefValue::instance());
            t->push(exception);

            frame_stack->pop_back();

            stack.push(t);

            break;
        } else {
            // printf("THIS IS NOT A FUNCTION FRAME\n");
            while (stack.size() > frame->top) {
                stack.pop();
            }
            frame_stack->pop_back();
        }
    }
}

static SharedPtr<Value> av_size(SharedPtr<Value>& self) {
    SharedPtr<IntValue> size = new IntValue(self->upcast<ArrayValue>()->size());
    return size;
}

static SharedPtr<Value> str_length(SharedPtr<Value>& self) {
    return new IntValue(self->upcast<StrValue>()->length());
}

static SharedPtr<Value> str_match(SharedPtr<Value>&self, SharedPtr<Value>&arg1) {
    SharedPtr<AbstractRegexpValue> regex = arg1->upcast<AbstractRegexpValue>();
    SharedPtr<BoolValue> b = new BoolValue(regex->match(self->upcast<StrValue>()->str_value));
    return b;
}

static SharedPtr<Value> builtin_p(SharedPtr<Value>&arg1) {
    arg1->dump();
    return UndefValue::instance();
}

static SharedPtr<Value> builtin_getenv(SharedPtr<Value> &v) {
    SharedPtr<Value> s(v->to_s());
    char *env = getenv(s->upcast<StrValue>()->str_value.c_str());
    if (env) {
        SharedPtr<StrValue> ret = new StrValue();
        ret->set_str(env);
        return ret;
    } else {
        return UndefValue::instance();
    }
}

static SharedPtr<Value> builtin_exit(SharedPtr<Value> &v) {
    assert(v->value_type == VALUE_TYPE_INT);
    SharedPtr<Value> s(v->to_i());
    exit(s->upcast<IntValue>()->int_value);
}

static SharedPtr<Value> eval_foo(VM *vm, std::istream * is, const std::string & package) {
    SharedPtr<Scanner> scanner(new Scanner(is));

    Node *yylval = NULL;
    int token_number;
    tora::Parser parser;
    do {
        token_number = scanner->scan(&yylval);
        parser.set_lineno(scanner->lineno());
        parser.parse(token_number, yylval);
    } while (token_number != 0);
    if (scanner->in_heredoc()) {
        return new ExceptionValue("Unexpected EOF in heredoc.");
    }
    if (parser.is_failure()) {
        return new ExceptionValue("Parsing failed.");
    }

    // compile
    // SharedPtr<SymbolTable> symbol_table = new SymbolTable();
    Compiler compiler(vm->symbol_table);
    compiler.init_globals();
    compiler.package(package);
    compiler.compile(parser.root_node());
    if (compiler.error) {
        return new ExceptionValue("Compilation failed.");
    }

    // run it
    // tora::VM vm(compiler.ops, symbol_table);
    // vm->init_globals(argc-optind, argv+optind);

    // save orig ops.
    SharedPtr<OPArray> orig_ops = vm->ops;
    int orig_pc = vm->pc;
    size_t orig_stack_size = vm->stack.size();
    size_t orig_frame_size = vm->frame_stack->size();

    if (0) {
        Disasm::disasm(compiler.ops);
    }

    vm->ops = compiler.ops;
    vm->pc = 0;
    vm->execute();

    // restore
    vm->ops= orig_ops;
    vm->pc = orig_pc;

    // remove frames
    while (orig_frame_size < vm->frame_stack->size()) {
        vm->frame_stack->pop_back();
    }

    if (orig_stack_size < vm->stack.size()) {
        SharedPtr<Value> ret = vm->stack.pop();
        while (orig_stack_size < vm->stack.size()) {
            vm->stack.pop();
        }
        return ret;
    } else {
        return UndefValue::instance();
    }
}
static SharedPtr<Value> eval_foo(VM *vm, std::istream * is) {
    // TODO use current vm's package
    return eval_foo(vm, is, "main");
}

static SharedPtr<Value> do_foo(VM *vm, const std::string &fname, std::string & package) {
    std::ifstream *ifs = new std::ifstream(fname.c_str());
    if (ifs->is_open()) {
        return eval_foo(vm, ifs, package);
    } else {
        return new ExceptionValue(fname + " : " + strerror(errno));
    }
}

static SharedPtr<Value> builtin_eval(VM * vm, SharedPtr<Value> &v) {
    assert(v->value_type == VALUE_TYPE_STR);

    std::stringstream *ss = new std::stringstream(v->upcast<StrValue>()->str_value + ";");
    return eval_foo(vm, ss);
}

/**
 * do $file; => eval(open($file).read())
 */
static SharedPtr<Value> builtin_do(VM * vm, SharedPtr<Value> &v) {
    assert(v->value_type == VALUE_TYPE_STR);
    std::ifstream *ifs = new std::ifstream(v->upcast<StrValue>()->str_value.c_str(), std::ios::in);
    if (ifs->is_open()) {
        return eval_foo(vm, ifs);
    } else {
        return new ExceptionValue(v->upcast<StrValue>()->str_value + " : " + strerror(errno));
    }
}

SharedPtr<Value> VM::require(SharedPtr<Value> &v) {
    VM *vm = this;
    SharedPtr<ArrayValue> libpath = vm->global_vars->at(2)->upcast<ArrayValue>();
    SharedPtr<HashValue> required = vm->global_vars->at(3)->upcast<HashValue>();
    std::string s = v->to_s()->str_value;
    std::string package = v->to_s()->str_value;
    {
        auto iter = s.find("::");
        while (iter != std::string::npos) {
            s.replace(iter, 2, "/");
            iter = s.find("::");
        }
        s += ".tra";
    }

    // inc check
    if (required->has_key(s)) {
        if (required->get(s)->value_type == VALUE_TYPE_UNDEF) {
            return new StrValue("Compilation failed in require");
        } else {
            return new IntValue(1);
        }
    }

    // load
    for (int i=0; i<libpath->size(); i++) {
        std::string realfilename;
        realfilename = libpath->at(i)->to_s()->str_value;
        realfilename += "/";
        realfilename += s;
        struct stat stt;
        if (stat(realfilename.c_str(), &stt)==0) {
            SharedPtr<Value> realfilename_value(new StrValue(realfilename));
            required->set_item(new StrValue(s), realfilename_value);
            SharedPtr<Value> ret = do_foo(vm, realfilename, package);
            if (ret->value_type == VALUE_TYPE_EXCEPTION) {
                required->set_item(new StrValue(s), UndefValue::instance());
                return ret;
            } else {
                return ret;
            }
        }
    }

    // not found...
    std::string message = std::string("Cannot find ") + s + " in $LIBPATH\n";
    for (int i=0; i<libpath->size(); i++) {
        message += "  " + libpath->at(i)->to_s()->str_value;
    }
    return new ExceptionValue(message);
}

/**
 */
static SharedPtr<Value> builtin_require(VM *vm, SharedPtr<Value> &v) {
    return vm->require(v);
}

static SharedPtr<Value> builtin_open(const std::vector<SharedPtr<Value>> & args) {
    SharedPtr<Value> filename(args.at(1));
    std::string mode;
    if (args.size() >= 2) {
        mode = args.at(1)->upcast<StrValue>()->str_value.c_str();
    } else {
        mode = "rb";
    }
    // TODO: check \0
    SharedPtr<FileValue> file = new FileValue();
    if (file->open(
        filename->upcast<StrValue>()->str_value,
        mode
    )) {
        return file;
    } else {
        abort(); // todo: throw exception
    }
}


static SharedPtr<Value> builtin_print(const std::vector<SharedPtr<Value>> & args) {
    auto iter = args.begin();
    for (; iter!=args.end(); iter++) {
        SharedPtr<Value> v(*iter);
        SharedPtr<Value> s(v->to_s());
        printf("%s", s->upcast<StrValue>()->str_value.c_str());
    }
    return UndefValue::instance();
}

static SharedPtr<Value> builtin_say(const std::vector<SharedPtr<Value>> & args) {
    auto iter = args.begin();
    for (; iter!=args.end(); iter++) {
        SharedPtr<Value> v(*iter);
        SharedPtr<Value> s(v->to_s());
        // printf("%s\n", s->upcast<StrValue>()->str_value.c_str());
        fwrite(s->upcast<StrValue>()->str_value.c_str(), sizeof(char), s->upcast<StrValue>()->str_value.size(), stdout);
        fputc('\n', stdout);
    }
    return UndefValue::instance();
}

static SharedPtr<Value> builtin_package(VM *vm) {
    return new StrValue(vm->package);
}

void VM::register_standard_methods() {
    {
        MetaClass meta(this, VALUE_TYPE_ARRAY);
        meta.add_method("size", av_size);
    }
    {
        MetaClass meta(this, VALUE_TYPE_STR);
        meta.add_method("length", str_length);
        meta.add_method("match", str_match);
    }
    // this->add_builtin_function("print");
    this->add_builtin_function("p", builtin_p);
    this->add_builtin_function("getenv", builtin_getenv);
    this->add_builtin_function("exit", builtin_exit);
    this->add_builtin_function("say", builtin_say);
    this->add_builtin_function("open", builtin_open);
    this->add_builtin_function("print", builtin_print);
    this->add_builtin_function("eval", builtin_eval);
    this->add_builtin_function("do",   builtin_do);
    this->add_builtin_function("require",   builtin_require);
    this->add_builtin_function("__PACKAGE__",   builtin_package);
}

SharedPtr<Value> VM::copy_all_public_symbols(const std::string &src, const std::string &dst) {
    ID srcid = this->symbol_table->get_id(src);
    ID dstid = this->symbol_table->get_id(dst);
    SharedPtr<Package> srcpkg = this->find_package(srcid);
    SharedPtr<Package> dstpkg = this->find_package(dstid);

    // printf("Copying %s to %s\n", src.c_str(), dst.c_str());
    auto iter = srcpkg->begin();
    for (; iter!=srcpkg->end(); iter++) {
        SharedPtr<Value> v = iter->second;
        if (v->value_type == VALUE_TYPE_CODE) {
            dstpkg->add_function(v->upcast<CodeValue>()->func_name_id, v);
        } else {
            // copy non-code value to other package?
            abort();
        }
    }
    // dstpkg->dump(this->symbol_table, 1);
    return UndefValue::instance();
}

void VM::add_function(ID id, SharedPtr<Value> code) {
    this->find_package(this->package_id())->add_function(id, code);
}

void Package::add_function(ID function_name_id, SharedPtr<Value> code) {
    this->data[function_name_id] = code;
}

SharedPtr<Package> VM::find_package(ID id) {
    auto iter = this->package_map->find(id);
    if (iter != this->package_map->end()) {
        return iter->second;
    } else {
        SharedPtr<Package> pkg =  new Package(id);
        this->package_map->set(pkg);
        return pkg;
    }
}

