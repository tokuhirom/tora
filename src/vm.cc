#include "vm.h"
#include "value.h"
#include "tora.h"
#include "frame.h"

#include "value/hash.h"
#include "value/code.h"
#include "value/regexp.h"
#include "value/file.h"
#include "value/symbol.h"
#include "value/pointer.h"
#include "value/array.h"
#include "value/tuple.h"
#include "value/object.h"

#include "object/array.h"
#include "object/str.h"
#include "object/dir.h"
#include "object/stat.h"
#include "object/env.h"
#include "object/json.h"
#include "object/time.h"
#include "object/file.h"
#include "object/socket.h"

#include <boost/foreach.hpp>
#include <sys/types.h>
#include <dirent.h>
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
#include <random>

#include <boost/random.hpp>
#include <boost/scoped_ptr.hpp>

using namespace tora;

const int INITIAL_STACK_SIZE = 1024;

VM::VM(SharedPtr<OPArray>& ops_, SharedPtr<SymbolTable> &symbol_table_) : ops(ops_), symbol_table(symbol_table_), stack(INITIAL_STACK_SIZE), exec_trace(false) {
    sp = 0;
    pc = 0;
    this->frame_stack = new std::vector<LexicalVarsFrame*>();
    this->frame_stack->push_back(new LexicalVarsFrame(0));
    this->global_vars = new std::vector<SharedPtr<Value>>();
    this->package_map = new PackageMap();
    this->package_id(symbol_table_->get_id("main"));
    this->myrand = new boost::mt19937(time(NULL));
    this->mark_stack.reserve(1024);
}

VM::~VM() {
    delete this->global_vars;
    // assert(this->frame_stack->size() == 1);
    delete this->frame_stack->back();
    delete this->frame_stack;
    delete this->myrand;

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
    SharedPtr<ObjectValue> env = new ObjectValue(this->symbol_table->get_id("Env"), this);
    this->global_vars->push_back(env);

    // $LIBPATH : Array
    SharedPtr<ArrayValue> libpath = new ArrayValue();
    libpath->push(new StrValue("lib"));
    this->global_vars->push_back(libpath);

    // $REQUIRED : Hash
    this->global_vars->push_back(new HashValue());
}

template <class operationI, class operationD>
void tora::VM::binop(operationI operation_i, operationD operation_d) {
    SharedPtr<Value> v1(stack.back()); /* rvalue */
    stack.pop_back();
    SharedPtr<Value> v2(stack.back()); /* lvalue */
    stack.pop_back();

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

template void tora::VM::binop(std::multiplies<int> operation_i, std::multiplies<double> operation_d);
template void tora::VM::binop(std::minus<int> operation_i, std::minus<double> operation_d);
template void tora::VM::binop(std::divides<int> operation_i, std::divides<double> operation_d);

// TODO: return SharedPtr<Value>
template <class operationI, class operationD, class OperationS>
SharedPtr<Value> VM::cmpop(operationI operation_i, operationD operation_d, OperationS operation_s) {
    SharedPtr<Value> v1(stack.back());
    stack.pop_back();
    SharedPtr<Value> v2(stack.back());
    stack.pop_back();
 
    switch (v1->value_type) {
    case VALUE_TYPE_INT: {
        Value * ie2 = v2->to_int();
        if (ie2->is_exception()) { return ie2; }
        SharedPtr<BoolValue> result = BoolValue::instance(operation_i(v1->upcast<IntValue>()->int_value, ie2->upcast<IntValue>()->int_value));
        return result;
    }
    case VALUE_TYPE_STR: {
        SharedPtr<Value> s2(v2->to_s());
        if (s2->is_exception()) { return s2; }
        SharedPtr<BoolValue> result = BoolValue::instance(operation_s(v1->upcast<StrValue>()->str_value, s2->upcast<StrValue>()->str_value));
        return result;
    }
    case VALUE_TYPE_DOUBLE: {
        switch (v2->value_type) {
        case VALUE_TYPE_INT: {
            SharedPtr<BoolValue> result = BoolValue::instance(operation_d(v1->upcast<DoubleValue>()->double_value, (double)v2->upcast<IntValue>()->int_value));
            return result;
        }
        case VALUE_TYPE_DOUBLE: {
            SharedPtr<BoolValue> result = BoolValue::instance(operation_d(v1->upcast<DoubleValue>()->double_value, v2->upcast<DoubleValue>()->double_value));
            return result;
        }
        default: {
            TODO(); // throw exception
            abort();
        }
        }
        break;
    }
    case VALUE_TYPE_UNDEF: {
        return new BoolValue(v2->value_type == VALUE_TYPE_UNDEF);
    }
    default:
        // TODO: support object comparation
        return new ExceptionValue("UNKNOWN MATCHING PATTERN:: %s\n", opcode2name[ops->at(pc)->op_type]);
    }
    abort();
}

template SharedPtr<Value> VM::cmpop(std::equal_to<int>, std::equal_to<double>, std::equal_to<std::string>);
template SharedPtr<Value> VM::cmpop(std::greater<int>, std::greater<double>, std::greater<std::string>);
template SharedPtr<Value> VM::cmpop(std::greater_equal<int>, std::greater_equal<double>, std::greater_equal<std::string>);
template SharedPtr<Value> VM::cmpop(std::less<int>, std::less<double>, std::less<std::string>);
template SharedPtr<Value> VM::cmpop(std::less_equal<int>, std::less_equal<double>, std::less_equal<std::string>);
template SharedPtr<Value> VM::cmpop(std::not_equal_to<int>, std::not_equal_to<double>, std::not_equal_to<std::string>);

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
            } else if (exception->value_type == VALUE_TYPE_EXCEPTION) {
                if (exception->upcast<ExceptionValue>()->exception_type == EXCEPTION_TYPE_GENERAL) {
                    fprintf(stderr, "%s\n", exception->upcast<ExceptionValue>()->message().c_str());
                } else if (exception->upcast<ExceptionValue>()->exception_type == EXCEPTION_TYPE_ERRNO) {
                    fprintf(stderr, "%s\n", strerror(exception->upcast<ExceptionValue>()->get_errno()));
                } else {
                    TODO();
                }
            } else {
                fprintf(stderr, "died\n");
                exception->dump();
            }
            exit(1);
            break;
        }

        boost::scoped_ptr<LexicalVarsFrame> frame(frame_stack->back());
        if (frame->type == FRAME_TYPE_FUNCTION) {
            FunctionFrame* fframe = static_cast<FunctionFrame*>(frame.get());
            pc = fframe->return_address;
            ops = fframe->orig_ops;

            stack.resize(frame->top);

            frame_stack->pop_back();
        } else if (frame->type == FRAME_TYPE_TRY) {
            TryFrame* tframe = static_cast<TryFrame*>(frame.get());
            pc = tframe->return_address;

            stack.resize(frame->top);
            SharedPtr<TupleValue> t = new TupleValue();
            t->push(UndefValue::instance());
            t->push(exception);

            frame_stack->pop_back();

            stack.push(t);

            break;
        } else {
            // printf("THIS IS NOT A FUNCTION FRAME\n");
            stack.resize(frame->top);
            frame_stack->pop_back();
        }
    }
}

static SharedPtr<Value> builtin_p(VM *vm, Value* arg1) {
    arg1->dump();
    return UndefValue::instance();
}

static SharedPtr<Value> builtin_exit(VM *vm, Value* v) {
    assert(v->value_type == VALUE_TYPE_INT);
    SharedPtr<Value> s(v->to_int());
    if (s->is_exception()) { return s; }
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
    if (vm->exec_trace) {
        vm->execute_trace();
    } else {
        vm->execute();
    }

    // restore
    vm->ops= orig_ops;
    vm->pc = orig_pc;

    // remove frames
    while (orig_frame_size < vm->frame_stack->size()) {
        delete vm->frame_stack->back();
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

static SharedPtr<Value> builtin_eval(VM * vm, Value* v) {
    assert(v->value_type == VALUE_TYPE_STR);

    std::stringstream *ss = new std::stringstream(v->upcast<StrValue>()->str_value + ";");
    return eval_foo(vm, ss);
}

/**
 * do $file; => eval(open($file).read())
 */
static SharedPtr<Value> builtin_do(VM * vm, Value *v) {
    assert(v->value_type == VALUE_TYPE_STR);
    std::ifstream *ifs = new std::ifstream(v->upcast<StrValue>()->str_value.c_str(), std::ios::in);
    if (ifs->is_open()) {
        return eval_foo(vm, ifs);
    } else {
        return new ExceptionValue(v->upcast<StrValue>()->str_value + " : " + strerror(errno));
    }
}

SharedPtr<Value> VM::require(Value * v) {
    VM *vm = this;
    SharedPtr<ArrayValue> libpath = vm->global_vars->at(2)->upcast<ArrayValue>();
    SharedPtr<HashValue> required = vm->global_vars->at(3)->upcast<HashValue>();
    std::string s = symbol_table->id2name(v->upcast<SymbolValue>()->id);
    std::string package = s;
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
static SharedPtr<Value> builtin_require(VM *vm, Value *v) {
    return vm->require(v);
}

/**
 * open(Str $fname) : FileHandle
 * open(Str $fname, Str $mode) : FileHandle
 */
static SharedPtr<Value> builtin_open(VM *vm, const std::vector<SharedPtr<Value>> & args) {
    if (args.size() != 1 && args.size() != 2) {
        return new ExceptionValue("Invalid argument count for open(): %zd. open() function requires 1 or 2 argument.", args.size());
    }

    SharedPtr<Value> filename(args.at(0));
    std::string mode;
    if (args.size() == 2) {
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
        return new ExceptionValue("Cannot open file: %s: %s", filename->upcast<StrValue>()->str_value.c_str(), strerror(errno));
    }
}


static SharedPtr<Value> builtin_print(VM *vm, const std::vector<SharedPtr<Value>> & args) {
    auto iter = args.begin();
    for (; iter!=args.end(); iter++) {
        SharedPtr<Value> v(*iter);
        SharedPtr<Value> s(v->to_s());
        printf("%s", s->upcast<StrValue>()->str_value.c_str());
    }
    return UndefValue::instance();
}

static SharedPtr<Value> builtin_say(VM *vm, const std::vector<SharedPtr<Value>> & args) {
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
    return new StrValue(vm->package_name());
}

static SharedPtr<Value> builtin_typeof(VM *vm, Value *v) {
    return new StrValue(v->type_str());
}

static SharedPtr<Value> builtin_self(VM *vm) {
    auto iter = vm->frame_stack->begin();
    for (; iter!=vm->frame_stack->end(); ++iter) {
        if ((*iter)->type == FRAME_TYPE_FUNCTION) {
            if ((*iter)->upcast<FunctionFrame>()->self) {
                return (*iter)->upcast<FunctionFrame>()->self;
            } else {
                return UndefValue::instance();
            }
        }
    }
    return new ExceptionValue("Cannot call 'self' method out of method.");
}

static SharedPtr<Value> builtin_opendir(VM * vm, Value* s) {
    SharedPtr<StrValue> dirname = s->to_s();
    DIR * dp = opendir(dirname->c_str());
    if (dp) {
        SharedPtr<ObjectValue> o = new ObjectValue(vm->symbol_table->get_id("Dir"), vm);
        o->set_value(vm->symbol_table->get_id("__d"), new PointerValue(dp));
        return o;
    } else {
        return UndefValue::instance();
    }
}

/**
 * rand() : Double;
 * rand($n Double) : Double;
 * rand($n Int)     : Int;
 *
 * Get a random number.
 *
 * rand() returns [0, 1) in Double value.
 * rand($n :Double) returns [0, $n) in Double value.
 * rand($n :Int) returns [0, $n) in Int value.
 */
static SharedPtr<Value> builtin_rand(VM *vm, const std::vector<SharedPtr<Value>>& args) {
    // TODO: use std::mt19937 in c++11.
    // in 2012, I use g++ 4.4.3. It does not support uniform_real_distribution
    if (args.size() == 0) {
        boost::uniform_real<double> dist(0.0, 1.0);
        return new DoubleValue(dist(*(vm->myrand)));
    } else if (args.size() == 1) {
        SharedPtr<Value> v = args.at(0);
        if (v->value_type == VALUE_TYPE_DOUBLE) {
            boost::uniform_real<double> dist(0.0, v->upcast<DoubleValue>()->double_value);
            return new DoubleValue(dist(*(vm->myrand)));
        } else if (v->value_type == VALUE_TYPE_INT) {
            boost::uniform_int<int> dist(0, v->upcast<IntValue>()->int_value);
            return new IntValue(dist(*(vm->myrand)));
        } else {
            // support to_int?
            return new ExceptionValue("Invalid arguments for rand() : %s", v->type_str());
        }
    } else {
        return new ExceptionValue("Too many arguments for rand()");
    }
}

#ifndef NDEBUG

static SharedPtr<Value> builtin_dump_stack(VM *vm) {
    vm->dump_stack();
    return UndefValue::instance();
}

#endif

void VM::call_native_func(const CallbackFunction* callback, int argcnt) {
    if (callback->argc==-1) {
        std::vector<SharedPtr<Value>> vec;
        for (int i=0; i<argcnt; i++) {
            SharedPtr<Value> arg = stack.pop();
            vec.push_back(arg);
        }
        SharedPtr<Value> ret = callback->func_vmv(this, vec);
        stack.push(ret);
    } else if (callback->argc==-2) {
        SharedPtr<Value> ret = callback->func_vm0(this);
        if (ret->value_type == VALUE_TYPE_EXCEPTION) {
            this->die(ret);
        } else {
            stack.push(ret);
        }
    } else if (callback->argc==-3) {
        SharedPtr<Value> v = stack.pop();
        SharedPtr<Value> ret = callback->func_vm1(this, v.get());
        if (ret->value_type == VALUE_TYPE_EXCEPTION && ret->upcast<ExceptionValue>()->exception_type != EXCEPTION_TYPE_STOP_ITERATION) {
            this->die(ret);
        } else {
            stack.push(ret);
        }
    } else if (callback->argc==-4) {
        SharedPtr<Value> v = stack.pop();
        SharedPtr<Value> v2 = stack.pop();
        SharedPtr<Value> ret = callback->func_vm2(this, v.get(), v2.get());
        if (ret->value_type == VALUE_TYPE_EXCEPTION) {
            this->die(ret);
        } else {
            stack.push(ret);
        }
    } else if (callback->argc == CallbackFunction::type_vm3) {
        SharedPtr<Value> v = stack.pop();
        SharedPtr<Value> v2 = stack.pop();
        SharedPtr<Value> v3 = stack.pop();
        SharedPtr<Value> ret = callback->func_vm3(this, v.get(), v2.get(), v3.get());
        if (ret->value_type == VALUE_TYPE_EXCEPTION) {
            this->die(ret);
        } else {
            stack.push(ret);
        }
    } else if (callback->argc == CallbackFunction::type_vm4) {
        SharedPtr<Value> v = stack.pop();
        SharedPtr<Value> v2 = stack.pop();
        SharedPtr<Value> v3 = stack.pop();
        SharedPtr<Value> v4 = stack.pop();
        SharedPtr<Value> ret = callback->func_vm4(this, v.get(), v2.get(), v3.get(), v4.get());
        if (ret->value_type == VALUE_TYPE_EXCEPTION) {
            this->die(ret);
        } else {
            stack.push(ret);
        }
    } else {
        abort();
    }
}


void VM::register_standard_methods() {
    Init_Array(this);
    Init_Str(this);
    Init_Dir(this);
    Init_Stat(this);
    Init_Env(this);
    Init_JSON(this);
    Init_Time(this);
    Init_File(this);
    Init_Socket(this);

    this->add_builtin_function("p", builtin_p);
    this->add_builtin_function("exit", builtin_exit);
    this->add_builtin_function("say", builtin_say);
    this->add_builtin_function("open", builtin_open);
    this->add_builtin_function("print", builtin_print);
    this->add_builtin_function("eval", builtin_eval);
    this->add_builtin_function("do",   builtin_do);
    this->add_builtin_function("require",   builtin_require);
    this->add_builtin_function("self",   builtin_self);
    this->add_builtin_function("__PACKAGE__",   builtin_package);
    this->add_builtin_function("opendir",   builtin_opendir);
    this->add_builtin_function("typeof",   builtin_typeof);
    this->add_builtin_function("rand",   builtin_rand);
#ifndef NDEBUG
    this->add_builtin_function("dump_stack",   builtin_dump_stack);
#endif
}

SharedPtr<Value> VM::copy_all_public_symbols(ID srcid, ID dstid) {
    SharedPtr<Package> srcpkg = this->find_package(srcid);
    SharedPtr<Package> dstpkg = this->find_package(dstid);

    // printf("Copying %d to %d\n", srcid, dstid);
    auto iter = srcpkg->begin();
    for (; iter!=srcpkg->end(); iter++) {
        SharedPtr<Value> v = iter->second;
        if (v->value_type == VALUE_TYPE_CODE) {
            // printf("Copying %d method\n", v->upcast<CodeValue>()->func_name_id);
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

// move package to package.h/package.cc
void Package::add_method(ID function_name_id, const CallbackFunction* code) {
    SharedPtr<CodeValue> cv = new CodeValue(code);
    cv->package_id = this->name_id;
    // printf("package!! %d::%d\n", name_id, function_name_id);
    this->data[function_name_id] = cv;
}

Package* VM::find_package(const char * name) {
    return this->find_package(this->symbol_table->get_id(name));
}

Package* VM::find_package(ID id) {
    auto iter = this->package_map->find(id);
    if (iter != this->package_map->end()) {
        return iter->second.get();
    } else {
        Package* pkg =  new Package(id);
        this->package_map->set(pkg);
        return pkg;
    }
}

void VM::add(SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs) {
    if (lhs->value_type == VALUE_TYPE_INT) {
        Value * ie = rhs->to_int();
        if (ie->is_exception()) { TODO(); }
        SharedPtr<IntValue> iv = ie->upcast<IntValue>();
        SharedPtr<IntValue>v = new IntValue(lhs->upcast<IntValue>()->int_value + iv->int_value);
        stack.push(v);
    } else if (lhs->value_type == VALUE_TYPE_STR) {
        // TODO: support null terminated string
        SharedPtr<StrValue>v = new StrValue();
        SharedPtr<Value> s(rhs->to_s());
        v->set_str(lhs->upcast<StrValue>()->str_value + s->upcast<StrValue>()->str_value);
        stack.push(v);
    } else if (lhs->value_type == VALUE_TYPE_DOUBLE) {
        TODO();
    } else {
        SharedPtr<Value> s(lhs->to_s());
        fprintf(stderr, "'%s' is not numeric or string.\n", s->upcast<StrValue>()->str_value.c_str());
        rhs->dump();
        lhs->dump();
        exit(1); // TODO : die
    }
}

SharedPtr<Value> VM::unary_negative(const SharedPtr<Value> & v) {
    switch (v->value_type) {
    case VALUE_TYPE_INT:
        return new IntValue(-(v->upcast<IntValue>()->int_value));
    case VALUE_TYPE_DOUBLE:
        return new DoubleValue(-(v->upcast<DoubleValue>()->double_value));
    default:
        return new ExceptionValue("%s is not a numeric. You cannot apply unary negative operator.\n", v->type_str());
    }
}

SharedPtr<Value> VM::set_item(const SharedPtr<Value>& container, const SharedPtr<Value>& index, const SharedPtr<Value>& rvalue) const {
    switch (container->value_type) {
    case VALUE_TYPE_OBJECT:
        return container->upcast<ObjectValue>()->set_item(index, rvalue);
    case VALUE_TYPE_HASH:
        return container->upcast<HashValue>()->set_item(index, rvalue);
    case VALUE_TYPE_ARRAY:
        return container->upcast<ArrayValue>()->set_item(index, rvalue);
    default:
        return new ExceptionValue("%s is not a container. You cannot store item.\n", container->type_str());
    }
}

void VM::dump_frame() {
    printf("-- dump frame             --\n");
    int i = 0;
    for (auto f = frame_stack->begin(); f != frame_stack->end(); f++) {
        printf("type: %s [%d]\n", (*f)->type_str(), i++);
        for (size_t n=0; n<(*f)->vars.size(); n++) {
            printf("  %zd\n", n);
            (*f)->vars.at(n)->dump();
        }
    }
    printf("---------------\n");
}

void VM::dump_stack() {
    printf("-- STACK DUMP --\nSP: %d\n", sp);
    for (size_t i=0; i<stack.size(); i++) {
        printf("[%zd] ", i);
        stack.at(i)->dump();
    }
    printf("----------------\n");
}

