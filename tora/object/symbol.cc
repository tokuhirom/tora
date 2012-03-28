#include <stdlib.h>

#include "../object.h"
#include "../vm.h"
#include "../package.h"
#include "../value/object.h"
#include "../value/symbol.h"

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
    return new ObjectValue(vm, klass->upcast<SymbolValue>()->id(), data);
}

void tora::Init_Symbol(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("Symbol");
    pkg->add_method(vm->symbol_table->get_id("bless"), new CallbackFunction(Symbol_bless));
}

