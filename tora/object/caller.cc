#include <stdlib.h>

#include "../object.h"
#include "../vm.h"
#include "../value/object.h"
#include "../value/class.h"
#include "../symbols.gen.h"

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
  return self->upcast<ObjectValue>()->data();
}

void tora::Init_Caller(VM* vm) {
  SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_CALLER_CLASS);
  klass->add_method(vm->symbol_table->get_id("code"),
                    new CallbackFunction(caller_code));
  vm->add_builtin_class(klass);
}
