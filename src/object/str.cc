#include "../vm.h"
#include "../value.h"
#include "../shared_ptr.h"
#include "str.h"
#include "../value/regexp.h"

using namespace tora;

static SharedPtr<Value> str_length(SharedPtr<Value>& self) {
    return new IntValue(self->upcast<StrValue>()->length());
}

static SharedPtr<Value> str_match(SharedPtr<Value>&self, SharedPtr<Value>&arg1) {
    SharedPtr<AbstractRegexpValue> regex = arg1->upcast<AbstractRegexpValue>();
    SharedPtr<BoolValue> b = new BoolValue(regex->match(self->upcast<StrValue>()->str_value));
    return b;
}

void tora::Init_Str(VM *vm) {
    MetaClass meta(vm, VALUE_TYPE_STR);
    meta.add_method("length", str_length);
    meta.add_method("match", str_match);
}
