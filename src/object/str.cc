#include "../vm.h"
#include "../value.h"
#include "../shared_ptr.h"
#include "str.h"
#include "../value/regexp.h"

using namespace tora;

static SharedPtr<Value> str_length(VM *vm, Value* self) {
    return new IntValue(self->upcast<StrValue>()->length());
}

static SharedPtr<Value> str_match(VM *vm, Value* self, Value* arg1) {
    SharedPtr<AbstractRegexpValue> regex = arg1->upcast<AbstractRegexpValue>();
    SharedPtr<BoolValue> b = new BoolValue(regex->match(self->upcast<StrValue>()->str_value));
    return b;
}

void tora::Init_Str(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("String");
    pkg->add_method(vm->symbol_table->get_id("length"), new CallbackFunction(str_length));
    pkg->add_method(vm->symbol_table->get_id("match"), new CallbackFunction(str_match));
}
