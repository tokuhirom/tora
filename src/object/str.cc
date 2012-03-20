#include "../object.h"
#include "../vm.h"
#include "../value.h"
#include "../shared_ptr.h"
#include "../value/regexp.h"
#include "../package.h"

using namespace tora;

static SharedPtr<Value> str_length(VM *vm, Value* self) {
    return new IntValue(self->upcast<StrValue>()->length());
}

static SharedPtr<Value> str_match(VM *vm, Value* self, Value* arg1) {
    SharedPtr<AbstractRegexpValue> regex = arg1->upcast<AbstractRegexpValue>();
    return new BoolValue(regex->match(self->upcast<StrValue>()->str_value));
}

/**
 * "foo".replace(/o/g, 'p') : Str
 * "foo".replace(/o/,  'p') : Str
 *
 * Replace string parts.
 */
static SharedPtr<Value> str_replace(VM *vm, Value* self, Value* arg1, Value *rewrite_v) {
    SharedPtr<AbstractRegexpValue> regex = arg1->upcast<AbstractRegexpValue>();
    int replacements;
    std::string ret = regex->replace(self->upcast<StrValue>()->str_value, rewrite_v->to_s()->str_value, replacements);
    return new StrValue(ret);
}

void tora::Init_Str(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("String");
    pkg->add_method(vm->symbol_table->get_id("length"), new CallbackFunction(str_length));
    pkg->add_method(vm->symbol_table->get_id("match"), new CallbackFunction(str_match));
    pkg->add_method(vm->symbol_table->get_id("replace"), new CallbackFunction(str_replace));
}

