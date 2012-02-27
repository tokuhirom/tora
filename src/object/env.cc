#include "env.h"
#include "../vm.h"
#include <stdlib.h>

using namespace tora;

// TODO: delete $ENV['PATH']

// $ENV['hoge'] = 'foo';
static SharedPtr<Value> env_set(VM * vm, Value* self, Value*k, Value*v) {
    if (v->value_type == VALUE_TYPE_UNDEF) {
        unsetenv(k->to_s()->str_value.c_str());
    } else {
        setenv(
            k->to_s()->str_value.c_str(),
            v->to_s()->str_value.c_str(),
            1 // overwrite
        );
    }
    return UndefValue::instance();
}

// say($ENV['hoge']);
static SharedPtr<Value> env_get(VM * vm, Value* self, Value*k) {
    char *v = getenv(k->to_s()->str_value.c_str());
    if (v) {
        return new StrValue(v);
    } else {
        return UndefValue::instance();
    }
}

void tora::Init_Env(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("Env");
    pkg->add_method(vm->symbol_table->get_id("get"), new CallbackFunction(env_get));
    pkg->add_method(vm->symbol_table->get_id("__getitem__"), new CallbackFunction(env_get));
    pkg->add_method(vm->symbol_table->get_id("__setitem__"), new CallbackFunction(env_set));
}

