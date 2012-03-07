#include "vm.h"
#include "value.h"
#include "tora.h"
#include "frame.h"
#include "package_map.h"

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
#include "object/internals.h"

#include "builtin.h"

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
const int INITIAL_MARK_STACK_SIZE = 128;

VM::VM(SharedPtr<OPArray>& ops_, SharedPtr<SymbolTable> &symbol_table_) : ops(ops_), symbol_table(symbol_table_), stack(), exec_trace(false) {
    sp = 0;
    pc = 0;
    this->stack.reserve(INITIAL_STACK_SIZE);
    this->frame_stack = new std::vector<LexicalVarsFrame*>();
    this->frame_stack->push_back(new LexicalVarsFrame(0, 0));
    this->global_vars = new std::vector<SharedPtr<Value>>();
    this->package_map = new PackageMap();
    this->package_id(symbol_table_->get_id("main"));
    this->myrand = new boost::mt19937(time(NULL));
    this->mark_stack.reserve(INITIAL_MARK_STACK_SIZE);
}

VM::~VM() {
    stack.resize(0);

    delete this->global_vars;
    // assert(this->frame_stack->size() == 1);
    while (frame_stack->size()) {
        delete this->frame_stack->back();
        frame_stack->pop_back();
    }
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

/**
 * subtract lhs and rhs.
 * return value must be return by caller.
 */
Value * tora::VM::sub(const SharedPtr<Value>& lhs, const SharedPtr<Value> & rhs) {
    if (lhs->value_type == VALUE_TYPE_DOUBLE) {
        if (rhs->value_type == VALUE_TYPE_DOUBLE) {
            return new DoubleValue(lhs->upcast<DoubleValue>()->double_value - rhs->upcast<DoubleValue>()->double_value);
        } else if (rhs->value_type == VALUE_TYPE_INT) {
            return new DoubleValue(lhs->upcast<DoubleValue>()->double_value - (double)rhs->upcast<IntValue>()->int_value);
        } else {
            SharedPtr<Value> s(rhs->to_s());
            this->die("'%s' is not numeric.", s->upcast<StrValue>()->str_value.c_str());
        }
    } else if (lhs->value_type == VALUE_TYPE_INT) {
        IntValue* rhsi = rhs->to_int();
        return new IntValue(lhs->upcast<IntValue>()->int_value - rhsi->int_value);
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        this->die("'%s' is not numeric.", s->upcast<StrValue>()->str_value.c_str());
    }
    abort();
}

template <class operationI, class operationD>
void tora::VM::binop(operationI operation_i, operationD operation_d) {
    SharedPtr<Value> v1(stack.back()); /* rvalue */
    stack.pop_back();
    SharedPtr<Value> v2(stack.back()); /* lvalue */
    stack.pop_back();

    if (v2->value_type == VALUE_TYPE_DOUBLE) {
        if (v1->value_type == VALUE_TYPE_DOUBLE) {
            Value* v = new DoubleValue(operation_d(v2->upcast<DoubleValue>()->double_value, v1->upcast<DoubleValue>()->double_value));
            stack.push_back(v);
        } else if (v1->value_type == VALUE_TYPE_INT) {
            Value *v = new DoubleValue(operation_d(v2->upcast<DoubleValue>()->double_value, (double)v1->upcast<IntValue>()->int_value));
            stack.push_back(v);
        }
    } else if (v2->value_type == VALUE_TYPE_INT) {
        Value * v = new IntValue(operation_i(v2->upcast<IntValue>()->int_value, v1->upcast<IntValue>()->int_value));
        stack.push_back(v);
    } else { 
        SharedPtr<Value> s(v2->to_s());
        this->die("'%s' is not numeric.", s->upcast<StrValue>()->str_value.c_str());
    }
}

template void tora::VM::binop(std::multiplies<int> operation_i, std::multiplies<double> operation_d);
template void tora::VM::binop(std::divides<int> operation_i, std::divides<double> operation_d);

// TODO: return SharedPtr<Value>
template <class operationI, class operationD, class OperationS>
bool VM::cmpop(operationI operation_i, operationD operation_d, OperationS operation_s, const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs) {
 
    switch (lhs->value_type) {
    case VALUE_TYPE_INT: {
        SharedPtr<IntValue> ie2 = rhs->to_int();
        return operation_i(lhs->upcast<IntValue>()->int_value, ie2->int_value);
    }
    case VALUE_TYPE_STR: {
        SharedPtr<Value> s2(rhs->to_s());
        return (operation_s(lhs->upcast<StrValue>()->str_value, s2->upcast<StrValue>()->str_value));
    }
    case VALUE_TYPE_DOUBLE: {
        switch (rhs->value_type) {
        case VALUE_TYPE_INT: {
            return (operation_d(lhs->upcast<DoubleValue>()->double_value, (double)rhs->upcast<IntValue>()->int_value));
        }
        case VALUE_TYPE_DOUBLE: {
            return (operation_d(lhs->upcast<DoubleValue>()->double_value, rhs->upcast<DoubleValue>()->double_value));
        }
        default: {
            TODO(); // throw exception
            abort();
        }
        }
        break;
    }
    case VALUE_TYPE_BOOL: {
        return lhs->upcast<BoolValue>()->bool_value == rhs->to_bool();
    }
    case VALUE_TYPE_UNDEF: {
        return rhs->value_type == VALUE_TYPE_UNDEF;
    }
    default:
        // TODO: support object comparation
        throw new ExceptionValue("UNKNOWN MATCHING PATTERN:: %s\n", opcode2name[ops->at(pc)->op_type]);
    }
    abort();
}

template bool VM::cmpop(std::equal_to<int>, std::equal_to<double>, std::equal_to<std::string>, const SharedPtr<Value>&, const SharedPtr<Value> &);
template bool VM::cmpop(std::greater<int>, std::greater<double>, std::greater<std::string>, const SharedPtr<Value>&, const SharedPtr<Value> &);
template bool VM::cmpop(std::greater_equal<int>, std::greater_equal<double>, std::greater_equal<std::string>, const SharedPtr<Value>&, const SharedPtr<Value> &);
template bool VM::cmpop(std::less<int>, std::less<double>, std::less<std::string>, const SharedPtr<Value>&, const SharedPtr<Value> &);
template bool VM::cmpop(std::less_equal<int>, std::less_equal<double>, std::less_equal<std::string>, const SharedPtr<Value>&, const SharedPtr<Value> &);
template bool VM::cmpop(std::not_equal_to<int>, std::not_equal_to<double>, std::not_equal_to<std::string>, const SharedPtr<Value>&, const SharedPtr<Value> &);

void VM::die(const char *format, ...) {
    va_list ap;
    char p[4096+1];
    va_start(ap, format);
    vsnprintf(p, 4096, format, ap);
    va_end(ap);
    std::string s = p;
    throw new StrValue(s);
}

void VM::die(const SharedPtr<Value> & exception) {
    throw SharedPtr<Value>(exception);
}

static SharedPtr<Value> eval_foo(VM *vm, std::istream* is, const std::string & package, const std::string & fname) {
    Scanner scanner(is, fname);

    Node *yylval = NULL;
    int token_number;
    tora::Parser parser;
    do {
        token_number = scanner.scan(&yylval);
        parser.set_lineno(scanner.lineno());
        parser.parse(token_number, yylval);
    } while (token_number != 0);
    if (scanner.in_heredoc()) {
        throw new ExceptionValue("Unexpected EOF in heredoc.");
    }
    if (parser.is_failure()) {
        throw new ExceptionValue("Parsing failed.");
    }

    // compile
    // SharedPtr<SymbolTable> symbol_table = new SymbolTable();
    Compiler compiler(vm->symbol_table);
    compiler.init_globals();
    compiler.package(package);
    compiler.compile(parser.root_node());
    if (compiler.error) {
        throw new ExceptionValue("Compilation failed.");
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
        delete vm->frame_stack->back();
        vm->frame_stack->pop_back();
    }

    if (orig_stack_size < vm->stack.size()) {
        SharedPtr<Value> ret = vm->stack.back();
        vm->stack.pop_back();
        while (orig_stack_size < vm->stack.size()) {
            vm->stack.pop_back();
        }
        return ret;
    } else {
        return UndefValue::instance();
    }
}

static SharedPtr<Value> builtin_eval(VM * vm, Value* v) {
    assert(v->value_type == VALUE_TYPE_STR);

    std::stringstream ss(v->upcast<StrValue>()->str_value + ";");
    return eval_foo(vm, &ss, vm->package_name(), "<eval>");
}

/**
 * do $file; => eval(open($file).read())
 */
static SharedPtr<Value> builtin_do(VM * vm, Value *v) {
    assert(v->value_type == VALUE_TYPE_STR);
    SharedPtr<StrValue> fname = v->to_s();
    std::ifstream ifs(fname->str_value.c_str(), std::ios::in);
    if (ifs.is_open()) {
        return eval_foo(vm, &ifs, vm->package_name(), fname->str_value);
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
            std::ifstream ifs(realfilename.c_str());
            if (ifs.is_open()) {
                SharedPtr<Value> ret = eval_foo(vm, &ifs, package, realfilename).get();
                return ret;
            } else {
                required->set_item(new StrValue(s), UndefValue::instance());
                throw new ExceptionValue(realfilename + " : " + strerror(errno));
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

void VM::call_native_func(const CallbackFunction* callback, int argcnt) {
    if (callback->argc==-1) {
        std::vector<SharedPtr<Value>> vec;
        for (int i=0; i<argcnt; i++) {
            SharedPtr<Value> arg = stack.back();
            stack.pop_back();
            vec.push_back(arg);
        }
        SharedPtr<Value> ret = callback->func_vmv(this, vec);
        stack.push_back(ret);
    } else if (callback->argc==-2) {
        SharedPtr<Value> ret = callback->func_vm0(this);
        if (ret->value_type == VALUE_TYPE_EXCEPTION) {
            this->die(ret);
        } else {
            stack.push_back(ret);
        }
    } else if (callback->argc==-3) {
        SharedPtr<Value> v = stack.back();
        stack.pop_back();
        SharedPtr<Value> ret = callback->func_vm1(this, v.get());
        if (ret->value_type == VALUE_TYPE_EXCEPTION && ret->upcast<ExceptionValue>()->exception_type != EXCEPTION_TYPE_STOP_ITERATION) {
            this->die(ret);
        } else {
            stack.push_back(ret);
        }
    } else if (callback->argc==-4) {
        SharedPtr<Value> v = stack.back();
        stack.pop_back();
        SharedPtr<Value> v2 = stack.back();
        stack.pop_back();
        SharedPtr<Value> ret = callback->func_vm2(this, v.get(), v2.get());
        if (ret->value_type == VALUE_TYPE_EXCEPTION) {
            this->die(ret);
        } else {
            stack.push_back(ret);
        }
    } else if (callback->argc == CallbackFunction::type_vm3) {
        SharedPtr<Value> v = stack.back();
        stack.pop_back();
        SharedPtr<Value> v2 = stack.back();
        stack.pop_back();
        SharedPtr<Value> v3 = stack.back();
        stack.pop_back();
        SharedPtr<Value> ret = callback->func_vm3(this, v.get(), v2.get(), v3.get());
        if (ret->value_type == VALUE_TYPE_EXCEPTION) {
            this->die(ret);
        } else {
            stack.push_back(ret);
        }
    } else if (callback->argc == CallbackFunction::type_vm4) {
        SharedPtr<Value> v = stack.back();
        stack.pop_back();
        SharedPtr<Value> v2 = stack.back();
        stack.pop_back();
        SharedPtr<Value> v3 = stack.back();
        stack.pop_back();
        SharedPtr<Value> v4 = stack.back();
        stack.pop_back();
        SharedPtr<Value> ret = callback->func_vm4(this, v.get(), v2.get(), v3.get(), v4.get());
        if (ret->value_type == VALUE_TYPE_EXCEPTION) {
            this->die(ret);
        } else {
            stack.push_back(ret);
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
    Init_Internals(this);

    Init_builtins(this);

    this->add_builtin_function("eval", builtin_eval);
    this->add_builtin_function("do",   builtin_do);
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
        stack.push_back(v);
    } else if (lhs->value_type == VALUE_TYPE_STR) {
        // TODO: support null terminated string
        SharedPtr<StrValue>v = new StrValue();
        SharedPtr<Value> s(rhs->to_s());
        v->set_str(lhs->upcast<StrValue>()->str_value + s->upcast<StrValue>()->str_value);
        stack.push_back(v);
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

void VM::extract_tuple(const SharedPtr<TupleValue> &t) {
    int tuple_size = t->size();
    for (int i=0; i<tuple_size; i++) {
        this->stack.push_back(t->at(i));
    }
}

SharedPtr<Value> VM::get_self() {
    auto iter = this->frame_stack->begin();
    for (; iter!=this->frame_stack->end(); ++iter) {
        if ((*iter)->type == FRAME_TYPE_FUNCTION) {
            if ((*iter)->upcast<FunctionFrame>()->self) {
                return (*iter)->upcast<FunctionFrame>()->self;
            } else {
                return UndefValue::instance();
            }
        }
    }
    throw new ExceptionValue("Cannot call 'self' method out of method.");
}

void VM::package_id(ID id) {
    package_id_ = id;
    package_ = this->find_package(id);
}

void VM::handle_exception(const SharedPtr<Value> & exception) {
    assert(frame_stack->size() > 0);

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
                exception->dump(1);
            }
            exit(1);
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
            pc = tframe->return_address + 1;

            stack.resize(frame->top);
            SharedPtr<TupleValue> t = new TupleValue();
            t->push(UndefValue::instance());
            t->push(exception.get());

            frame_stack->pop_back();

            stack.push_back(t);

            break;
        } else {
            // printf("THIS IS NOT A FUNCTION FRAME\n");
            stack.resize(frame->top);
            frame_stack->pop_back();
        }
    }
}

void VM::execute() {
    bool next = false;
    do {
        try {
            if (exec_trace) {
                this->execute_trace();
            } else {
                this->execute_normal();
            }
            next = false;
        } catch (Value* exception) {
            SharedPtr<Value> v(exception);
            handle_exception(v);
            next = true;
        } catch (const SharedPtr<Value> &exception) {
            handle_exception(exception);
            next = true;
        };
    } while (next);
}

