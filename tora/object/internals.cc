#include "../object.h"
#include "../vm.h"
#include "../peek.h"
#include "../symbols.gen.h"
#include "../value/class.h"

using namespace tora;

/**
 * class Internals
 *
 * This is a utility functions for debugging tora itself.
 *
 * APIs may change without notice.
 */

/**
 * Internals.stack_size() : Int
 * 
 * Returns the number of items in stack.
 */
static SharedPtr<Value> stack_size(VM *vm, Value* self) {
    return new IntValue(vm->stack.size());
}

/**
 * Internals.dump_stack() : Undef
 *
 * Shows dump of stacks to stdout.
 * (Format may changes without notice.)
 */
static SharedPtr<Value> dump_stack(VM *vm) {
    vm->dump_stack();
    return UndefValue::instance();
}

/**
 * Internals.dump(Any $value); : Undef
 *
 * dump SV to stdout.
 */
static SharedPtr<Value> dump_dump(VM *vm, Value *self, Value *obj) {
    peek(vm, obj);
    return UndefValue::instance();
}

/**
 * Internals.dump_symbol_table() : Undef
 *
 * Dump symbol table to stdout.
 */
static SharedPtr<Value> dump_dump_symbol_table(VM *vm, Value *self) {
    vm->symbol_table->dump();
    return UndefValue::instance();
}

void tora::Init_Internals(VM* vm) {
    SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_INTERNALS_CLASS);
    klass->add_method("stack_size",        new CallbackFunction(stack_size));
    klass->add_method("dump_stack",        new CallbackFunction(dump_stack));
    klass->add_method("dump",              new CallbackFunction(dump_dump));
    klass->add_method("dump_symbol_table", new CallbackFunction(dump_dump_symbol_table));
    vm->add_builtin_class(klass);
}

