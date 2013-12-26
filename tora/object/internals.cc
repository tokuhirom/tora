#include "../object.h"
#include "../vm.h"
#include "../peek.h"
#include "../symbols.gen.h"
#include "../value/class.h"
#include "../class_builder.h"

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
static SharedPtr<Value> stack_size(VM *vm, Value *self) {
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
  return new_undef_value();
}

/**
 * Internals.dump(Any $value); : Undef
 *
 * dump SV to stdout.
 */
static SharedPtr<Value> dump_dump(VM *vm, Value *self, Value *obj) {
  peek(vm, obj);
  return new_undef_value();
}

/**
 * Internals.dump_symbol_table() : Undef
 *
 * Dump symbol table to stdout.
 */
static SharedPtr<Value> dump_dump_symbol_table(VM *vm, Value *self) {
  vm->symbol_table->dump();
  return new_undef_value();
}

void tora::Init_Internals(VM *vm) {
  ClassBuilder builder(vm, SYMBOL_INTERNALS_CLASS);
  builder.add_method("stack_size", new CallbackFunction(stack_size));
  builder.add_method("dump_stack", new CallbackFunction(dump_stack));
  builder.add_method("dump", new CallbackFunction(dump_dump));
  builder.add_method("dump_symbol_table",
                    new CallbackFunction(dump_dump_symbol_table));
  vm->add_builtin_class(builder.value());
}
