#include <sys/types.h>
#include <dirent.h>

#include "builtin.h"
#include "vm.h"
#include "value.h"
#include "value/file.h"
#include "value/object.h"
#include "value/pointer.h"

using namespace tora;

static SharedPtr<Value> builtin_exit(VM *vm, Value* v) {
    assert(v->value_type == VALUE_TYPE_INT);
    SharedPtr<Value> s(v->to_int());
    if (s->is_exception()) { return s; }
    exit(s->upcast<IntValue>()->int_value);
}

static SharedPtr<Value> builtin_p(VM *vm, Value* arg1) {
    arg1->dump();
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

static SharedPtr<Value> builtin_self(VM *vm) {
    return vm->get_self();
}

/**
 * require("foo/bar.tra")
 */
static SharedPtr<Value> builtin_require(VM *vm, Value *v) {
    return vm->require(v);
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
}
