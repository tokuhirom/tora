#include "../vm.h"
#include "internals.h"

using namespace tora;

/**
 * Internals.stack_size()
 * 
 */
static SharedPtr<Value> stack_size(VM *vm, Value* self) {
    return new IntValue(vm->stack.size());
}

void tora::Init_Internals(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package("Internals");
    pkg->add_method(vm->symbol_table->get_id("stack_size"), new CallbackFunction(stack_size));
}

