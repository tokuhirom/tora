#include "internals.h"
#include "../vm.h"
#include "../package.h"
#include "../peek.h"

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

/**
 * Internals.dump();
 *
 * dump SV.
 */
static SharedPtr<Value> dump_dump(VM *vm, Value *self, Value *obj) {
    peek(vm, obj);
    return UndefValue::instance();
}

static SharedPtr<Value> dump_dump_symbol_table(VM *vm, Value *self) {
    vm->symbol_table->dump();
    return UndefValue::instance();
}

void tora::Init_Internals(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package("Internals");
    pkg->add_method(vm->symbol_table->get_id("stack_size"), new CallbackFunction(stack_size));
    pkg->add_method(vm->symbol_table->get_id("dump_stack"), new CallbackFunction(dump_stack));
    pkg->add_method(vm->symbol_table->get_id("dump"), new CallbackFunction(dump_dump));
    pkg->add_method(vm->symbol_table->get_id("dump_symbol_table"), new CallbackFunction(dump_dump_symbol_table));
}

