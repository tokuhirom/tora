#include "internals.h"
#include "../vm.h"
#include "../package.h"

using namespace tora;

/**
 * Internals.stack_size()
 * 
 * Returns the number of items in stack.
 */
static SharedPtr<Value> stack_size(VM *vm, Value* self) {
    return new IntValue(vm->stack.size());
}

/**
 * Internals.dump_stack()
 *
 * Shows dump of stacks.
 * (Format may changes without notice.)
 */
static SharedPtr<Value> dump_stack(VM *vm) {
    vm->dump_stack();
    return UndefValue::instance();
}

void tora::Init_Internals(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package("Internals");
    pkg->add_method(vm->symbol_table->get_id("stack_size"), new CallbackFunction(stack_size));
    pkg->add_method(vm->symbol_table->get_id("dump_stack"), new CallbackFunction(dump_stack));
}

