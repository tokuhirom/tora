#include "env.h"
#include "../vm.h"
#include <stdlib.h>

using namespace tora;

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
}

