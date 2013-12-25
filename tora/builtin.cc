#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

#include "builtin.h"
#include "vm.h"
#include "value.h"
#include "frame.h"
#include "value/file.h"
#include "value/object.h"
#include "value/array.h"
#include "value/symbol.h"
#include "value/exception.h"
#include "value.h"
#include "object.h"
#include "inspector.h"
#include "printf.h"
#include "symbols.gen.h"

using namespace tora;

static SharedPtr<Value> builtin_exit(VM *vm, Value* v) {
    exit(v->to_int());
}

static SharedPtr<Value> builtin_p(VM *vm, Value* arg1) {
    Inspector ins(vm);
    assert(vm);
    assert(vm->symbol_table);
    printf("%s\n", ins.inspect(arg1).c_str());
    return new_undef_value();
}

static SharedPtr<Value> builtin_say(VM *vm, const std::vector<SharedPtr<Value>> & args) {
    auto iter = args.begin();
    for (; iter!=args.end(); iter++) {
        SharedPtr<Value> v(*iter);
        std::string s = v->to_s();
        // printf("%s\n", s->upcast<StrValue>()->str_value().c_str());
        fwrite(s.c_str(), sizeof(char), s.size(), stdout);
        fputc('\n', stdout);
    }
    return new_undef_value();
}

static SharedPtr<Value> builtin_typeof(VM *vm, Value *v) {
    return new_str_value(v->type_str());
}

/**
 * open(Str $fname) : FileHandle
 * open(Str $fname, Str $mode) : FileHandle
 */
static SharedPtr<Value> builtin_open(VM *vm, const std::vector<SharedPtr<Value>> & args) {
    if (args.size() != 1 && args.size() != 2) {
        throw new ExceptionValue("Invalid argument count for open(): %ld. open() function requires 1 or 2 argument.", (long int) args.size());
    }
    return File_open(vm, args.at(0).get(), args.size() == 2 ? args.at(1).get() : NULL);
}

static SharedPtr<Value> builtin_print(VM *vm, const std::vector<SharedPtr<Value>> & args) {
    auto iter = args.begin();
    for (; iter!=args.end(); iter++) {
        SharedPtr<Value> v(*iter);
        std::string s(v->to_s());
        printf("%s", s.c_str());
    }
    return new_undef_value();
}

static SharedPtr<Value> builtin_opendir(VM * vm, Value* s) {
    std::string dirname = s->to_s();
    return Dir_new(vm, dirname);
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
    if (args.size() == 0) {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return new DoubleValue(dist(*(vm->myrand)));
    } else if (args.size() == 1) {
        SharedPtr<Value> v = args.at(0);
        if (v->value_type == VALUE_TYPE_DOUBLE) {
            std::uniform_real_distribution<double> dist(-0.0, get_double_value(*v));
            return new DoubleValue(dist(*(vm->myrand)));
        } else if (v->value_type == VALUE_TYPE_INT) {
            std::uniform_int_distribution<int> dist(0, get_int_value(*v));
            return new IntValue(dist(*(vm->myrand)));
        } else {
            // support to_int?
            return new ExceptionValue("Invalid arguments for rand() : %s", v->type_str());
        }
    } else {
        return new ExceptionValue("Too many arguments for rand()");
    }
}

static SharedPtr<Value> builtin_self(VM *vm) {
    return vm->get_self();
}

/**
 * require("foo/bar.tra")
 */
static SharedPtr<Value> builtin_require(VM *vm, Value *v) {
    ID id = v->upcast<SymbolValue>()->id();
    std::string name = vm->symbol_table->id2name(id);
    vm->require_package(name);
    return new_undef_value();
}

/**
 * my $code = callee() : Code;
 *
 * Returns callee code object.
 * Return undef if it's not in subroutine.
 */
static SharedPtr<Value> builtin_callee(VM *vm) {
    for (auto iter = vm->frame_stack->rbegin(); iter != vm->frame_stack->rend(); ++iter) {
        if ((*iter)->type == FRAME_TYPE_FUNCTION) {
            FunctionFrame* fframe = (*iter)->upcast<FunctionFrame>();
            return fframe->code;
        }
    }
    return new_undef_value();
}

/**
 * caller(0) : Caller
 * caller() : Array[Caller]
 *
 */
static SharedPtr<Value> builtin_caller(VM *vm, const std::vector<SharedPtr<Value>>& args) {
    if (args.size() == 0) {
        int skiped_first = false;
        SharedPtr<ArrayValue> av = new ArrayValue();
        for (auto iter = vm->frame_stack->rbegin(); iter != vm->frame_stack->rend(); ++iter) {
            if ((*iter)->type == FRAME_TYPE_FUNCTION) {
                if (skiped_first) {
                    FunctionFrame* fframe = (*iter)->upcast<FunctionFrame>();
                    SharedPtr<ObjectValue> o = new ObjectValue(vm, vm->get_builtin_class(SYMBOL_CALLER_CLASS).get(), fframe->code);
                    av->push_back(o);
                } else {
                    skiped_first = true; // skip myself.
                }
            }
        }
        return av.get();
    } else if (args.size() == 1) {
        int seen = 0;
        int need = args.at(0)->to_int();
        for (auto iter = vm->frame_stack->rbegin(); iter != vm->frame_stack->rend(); ++iter) {
            if ((*iter)->type == FRAME_TYPE_FUNCTION) {
                FunctionFrame* fframe = (*iter)->upcast<FunctionFrame>();
                if (seen == need+1) {
                    SharedPtr<ObjectValue> o = new ObjectValue(vm, vm->get_builtin_class(SYMBOL_CALLER_CLASS).get(), fframe->code);
                    return o;
                }
                ++seen;
            }
        }
        return new_undef_value();
    } else {
        throw new ExceptionValue("Too many arguments for caller.");
    }
}

/**
 * Get a current working directory.
 */
static SharedPtr<Value> builtin_getcwd(VM *vm) {
    // getcwd() is POSIX.1-2001.
    // bad memory allocation??
    char * ptr = new char [MAXPATHLEN+1];
    if (getcwd(ptr, MAXPATHLEN)) {
        std::string ptr_s(ptr);
        delete ptr;
        return new_str_value(ptr_s);
    } else {
        throw new ErrnoExceptionValue(get_errno());
    }
}

/**
 * Get a current process id.
 */
static SharedPtr<Value> builtin_getpid(VM *vm) {
    // POSIX.1-2001, 4.3BSD, SVr4.
#ifdef _WIN32
    return new IntValue(-1);
#else
    return new IntValue(getpid());
#endif
}

/**
 * Get a parent process id.
 */
static SharedPtr<Value> builtin_getppid(VM *vm) {
    // POSIX.1-2001, 4.3BSD, SVr4.
#ifdef _WIN32
    return new IntValue(-1);
#else
    return new IntValue(getppid());
#endif
}

/**
 * system(Str $command); : Int
 *
 * Run the command with system(3).
 */
static SharedPtr<Value> builtin_system(VM *vm, Value * command) {
    // system(3) conforming to: C89, C99, POSIX.1-2001.
    return new IntValue(system(command->to_s().c_str()));
}

static SharedPtr<Value> builtin_abs(VM *vm, Value *v) {
    if (v->value_type == VALUE_TYPE_INT) {
        int i = v->to_int();
        return new IntValue(i < 0 ? -i : i);
    } else if (v->value_type == VALUE_TYPE_DOUBLE) {
        double i = v->to_double();
        return new DoubleValue(i < 0 ? -i : i);
    } else {
        throw new ExceptionValue("abs() is not supported in non numeric value.");
    }
}

static SharedPtr<Value> builtin_atan2(VM *vm, Value *y, Value *x) {
    return new DoubleValue(atan2(y->to_double(), x->to_double()));
}

static SharedPtr<Value> builtin_sqrt(VM *vm, Value *v) {
    return new DoubleValue(sqrt(v->to_double()));
}

static SharedPtr<Value> builtin_cos(VM *vm, Value *v) {
    return new DoubleValue(cos(v->to_double()));
}

static SharedPtr<Value> builtin_exp(VM *vm, Value *v) {
    return new DoubleValue(exp(v->to_double()));
}

/**
 * hex(Str $v) : Int
 *
 * hex('0xAF') # => 175
 * hex('aF') # => 175
 */
static SharedPtr<Value> builtin_hex(VM *vm, Value *v) {
    if (v->value_type != VALUE_TYPE_STR) {
        throw new ExceptionValue("hex() requires string value.");
    }
    const std::string s = get_str_value(v)->c_str();
    char *endp;
    set_errno(0);
    long n = strtol(s.c_str(), &endp, 16);
    if (get_errno() == ERANGE) {
        throw new ErrnoExceptionValue(get_errno());
    }
    if (endp != s.c_str()+s.size()) {
        throw new ExceptionValue("The value is not hexadecimal: %s.", get_str_value(v)->c_str());
    }
    return new IntValue(n);
}

static SharedPtr<Value> builtin_oct(VM *vm, Value *v) {
    if (v->value_type != VALUE_TYPE_STR) {
        throw new ExceptionValue("oct() requires string value.");
    }
    const std::string s = get_str_value(v)->c_str();
    char *endp;
    set_errno(0);
    long n = strtol(s.c_str(), &endp, 8);
    if (get_errno() == ERANGE) {
        throw new ErrnoExceptionValue(get_errno());
    }
    if (endp != s.c_str()+s.size()) {
        throw new ExceptionValue("The value is not oct: %s.", get_str_value(v)->c_str());
    }
    return new IntValue(n);
}

static SharedPtr<Value> builtin_int(VM *vm, Value *v) {
    return new IntValue(v->to_int());
}

static SharedPtr<Value> builtin_log(VM *vm, Value *v) {
    return new DoubleValue(log(v->to_double()));
}

static SharedPtr<Value> builtin_sin(VM *vm, Value *v) {
    return new DoubleValue(sin(v->to_double()));
}

static SharedPtr<Value> builtin_printf(VM *vm, const std::vector<SharedPtr<Value>> & args) {
    tora_printf(args);
    return new_undef_value();
}

static SharedPtr<Value> builtin_sprintf(VM *vm, const std::vector<SharedPtr<Value>> & args) {
    return new_str_value(tora_sprintf(args));
}

void tora::Init_builtins(VM *vm) {
#define ADD(name, cb) vm->add_builtin_function(name, new CallbackFunction(cb));
    ADD("p",         builtin_p);
    ADD("exit",      builtin_exit);
    ADD("say",       builtin_say);
    ADD("typeof",    builtin_typeof);
    ADD("open",      builtin_open);
    ADD("print",     builtin_print);
    ADD("require",   builtin_require);
    ADD("self",      builtin_self);
    ADD("opendir",   builtin_opendir);
    ADD("rand",      builtin_rand);
    ADD("caller",    builtin_caller);
    ADD("callee",    builtin_callee);
    ADD("getcwd",    builtin_getcwd);
    ADD("getpid",    builtin_getpid);
    ADD("getppid",   builtin_getppid);
    ADD("system",    builtin_system);
    ADD("printf",    builtin_printf);
    ADD("sprintf",   builtin_sprintf);
    
    // numeric functions
    ADD("sqrt",    builtin_sqrt);
    ADD("abs",     builtin_abs);
    ADD("atan2",   builtin_atan2);
    ADD("cos",     builtin_cos);
    ADD("exp",     builtin_exp);
    ADD("hex",     builtin_hex);
    ADD("int",     builtin_int);
    ADD("log",     builtin_log);
    ADD("oct",     builtin_oct);
    ADD("sin",     builtin_sin);
#undef ADD
}

