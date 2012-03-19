#include <sys/types.h>
#include <dirent.h>

#include "builtin.h"
#include "vm.h"
#include "value.h"
#include "frame.h"
#include "value/file.h"
#include "value/object.h"
#include "value/pointer.h"
#include "value/array.h"
#include "value/str.h"
#include "value/exception.h"
#include "value.h"
#include "object.h"
#include "inspector.h"

using namespace tora;

static SharedPtr<Value> builtin_exit(VM *vm, Value* v) {
    exit(v->to_int());
}

static SharedPtr<Value> builtin_p(VM *vm, Value* arg1) {
    Inspector ins(vm);
    assert(vm);
    assert(vm->symbol_table);
    printf("%s\n", ins.inspect(arg1).c_str());
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

static SharedPtr<Value> builtin_opendir(VM * vm, Value* s) {
    SharedPtr<StrValue> dirname = s->to_s();
    return Dir_new(vm, dirname.get());
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

static SharedPtr<Value> builtin_self(VM *vm) {
    return vm->get_self();
}

/**
 * require("foo/bar.tra")
 */
static SharedPtr<Value> builtin_require(VM *vm, Value *v) {
    return vm->require(v);
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
    return UndefValue::instance();
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
                    SharedPtr<ObjectValue> o = new ObjectValue(vm, vm->symbol_table->get_id("Caller"), fframe->code);
                    av->push(o);
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
                    SharedPtr<ObjectValue> o = new ObjectValue(vm, vm->symbol_table->get_id("Caller"), fframe->code);
                    return o;
                }
                ++seen;
            }
        }
        return UndefValue::instance();
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
        return new StrValue(ptr_s);
    } else {
        throw new ExceptionValue(errno);
    }
}

/**
 * Get a current process id.
 */
static SharedPtr<Value> builtin_getpid(VM *vm) {
    // POSIX.1-2001, 4.3BSD, SVr4.
    return new IntValue(getpid());
}

/**
 * Get a parent process id.
 */
static SharedPtr<Value> builtin_getppid(VM *vm) {
    // POSIX.1-2001, 4.3BSD, SVr4.
    return new IntValue(getppid());
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
    const std::string s = v->upcast<StrValue>()->c_str();
    char *endp;
    errno = 0;
    long n = strtol(s.c_str(), &endp, 16);
    if (errno == ERANGE) {
        throw new ExceptionValue(errno);
    }
    if (endp != s.c_str()+s.size()) {
        throw new ExceptionValue("The value is not hexadecimal: %s.", v->upcast<StrValue>()->c_str());
    }
    return new IntValue(n);
}

static SharedPtr<Value> builtin_oct(VM *vm, Value *v) {
    if (v->value_type != VALUE_TYPE_STR) {
        throw new ExceptionValue("oct() requires string value.");
    }
    const std::string s = v->upcast<StrValue>()->c_str();
    char *endp;
    errno = 0;
    long n = strtol(s.c_str(), &endp, 8);
    if (errno == ERANGE) {
        throw new ExceptionValue(errno);
    }
    if (endp != s.c_str()+s.size()) {
        throw new ExceptionValue("The value is not oct: %s.", v->upcast<StrValue>()->c_str());
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

void tora::Init_builtins(VM *vm) {
    vm->add_builtin_function("p", builtin_p);
    vm->add_builtin_function("exit", builtin_exit);
    vm->add_builtin_function("say", builtin_say);
    vm->add_builtin_function("__PACKAGE__",   builtin_package);
    vm->add_builtin_function("typeof",   builtin_typeof);
    vm->add_builtin_function("open", builtin_open);
    vm->add_builtin_function("print", builtin_print);
    vm->add_builtin_function("require",   builtin_require);
    vm->add_builtin_function("self",   builtin_self);
    vm->add_builtin_function("opendir",   builtin_opendir);
    vm->add_builtin_function("rand",   builtin_rand);
    vm->add_builtin_function("caller",   builtin_caller);
    vm->add_builtin_function("callee",   builtin_callee);
    vm->add_builtin_function("getcwd",   builtin_getcwd);
    vm->add_builtin_function("getpid",   builtin_getpid);
    vm->add_builtin_function("getppid",   builtin_getppid);
    
    // numeric functions
    vm->add_builtin_function("sqrt",   builtin_sqrt);
    vm->add_builtin_function("abs",   builtin_abs);
    vm->add_builtin_function("atan2",   builtin_atan2);
    vm->add_builtin_function("cos",   builtin_cos);
    vm->add_builtin_function("exp",   builtin_exp);
    vm->add_builtin_function("hex",   builtin_hex);
    vm->add_builtin_function("int",   builtin_int);
    vm->add_builtin_function("log",   builtin_log);
    vm->add_builtin_function("oct",   builtin_oct);
    vm->add_builtin_function("sin",   builtin_sin);
}

