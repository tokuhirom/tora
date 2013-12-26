#include <stdlib.h>

#include "../object.h"
#include "../vm.h"
#include "../value/object.h"
#include "../value/class.h"
#include "../symbols.gen.h"
#include "../class_builder.h"

using namespace tora;

/**
 * class Caller
 *
 * class Caller is a Caller object.
 * Caller class is returned from caller() builtin function.
 */

/**
 * $caller.code() : Code
 *
 * Return the caller code object.
 */
static SharedPtr<Value> caller_code(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_OBJECT);
  return object_data(self);
}

void tora::Init_Caller(VM* vm) {
  ClassBuilder builder(vm, SYMBOL_CALLER_CLASS);
  builder.add_method("code", new CallbackFunction(caller_code));
  vm->add_builtin_class(builder.value());
}
