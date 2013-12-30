#include "../object.h"
#include "../vm.h"
#include "../value/array.h"
#include "../value/object.h"
#include "../value/code.h"
#include "../value/class.h"
#include "../value/int.h"
#include "../peek.h"
#include "../symbols.gen.h"
#include "../class_builder.h"

using namespace tora;

/**
 * class Class
 *
 * This is a Class class.
 */

/**
 * $meta.name() : String
 *
 * Get a name of class.
 */
static SharedPtr<Value> Class_name(VM* vm, Value* self) {
  MortalStrValue s(class_name(self));
  return s.get();
}

void tora::Init_Class(VM* vm) {
  ClassBuilder builder(vm, SYMBOL_CLASS_CLASS);
  builder.add_method("name", std::make_shared<CallbackFunction>(Class_name));
  vm->add_builtin_class(builder.value());
}
