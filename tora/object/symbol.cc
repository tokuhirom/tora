#include <stdlib.h>

#include "../object.h"
#include "../vm.h"
#include "../symbols.gen.h"
#include "../value/object.h"
#include "../value/symbol.h"
#include "../value/class.h"

using namespace tora;

/**
 * class Symbol
 *
 * The symbol class.
 */

/**
 * $symbol.bless($data)
 *
 * Create a new object from $symbol and $data.
 */
SharedPtr<Value> tora::Symbol_bless(VM * vm, Value* klass, Value *data) {
    assert(klass->value_type == VALUE_TYPE_SYMBOL);
    return new ObjectValue(vm, vm->get_class(klass->upcast<SymbolValue>()->id()).get(), data);
}

void tora::Init_Symbol(VM *vm) {
    SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_SYMBOL_CLASS);
    klass->add_method(vm->symbol_table->get_id("bless"), new CallbackFunction(Symbol_bless));
    vm->add_builtin_class(klass);
}

