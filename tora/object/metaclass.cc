#include "../object.h"
#include "../vm.h"
#include "../value/array.h"
#include "../value/object.h"
#include "../value/code.h"
#include "../value/class.h"
#include "../peek.h"
#include "../symbols.gen.h"
#include "../class_builder.h"

using namespace tora;

/**
 * class MetaClass
 *
 * This is a metaclass. Meta class is class of class.
 *
 * You can get a meta class for $foo from $foo.meta().
 *
 * Note. API is compatible with Moose in Perl5, preferably.
 */

inline static Value* SELF(Value* self) {
  const SharedPtr<Value>& dat = self->upcast<ObjectValue>()->data();
  return dat->upcast<Value>();
}

/**
 * $meta.has_method(Str $name) : Boolean
 *
 * Returns a boolean indicating whether or not the class defines the named
 *method.
 * It does not include methods inherited from parent classes.
 */
static SharedPtr<Value> mc_has_method(VM* vm, Value* self, Value* methname_v) {
  std::string methname = methname_v->to_s();
  bool ret = class_has_method(SELF(self), vm->symbol_table->get_id(methname));
  return vm->to_bool(ret);
}

/**
 * $meta.get_method_list() : Array[Str]
 *
 * Get a method list defined in package.
 */
static SharedPtr<Value> mc_get_method_list(VM* vm, Value* self) {
  return class_get_method_list(SELF(self)).get();
}

/**
 * $meta.name() : String
 *
 * Get a name of class.
 */
static SharedPtr<Value> mc_name(VM* vm, Value* self) {
  MortalStrValue s(class_name(SELF(self)));
  return s.get();
}

/**
 * $meta.superclass() : String
 *
 * This method returns superclass in string.
 * If the class does not have a superclass, it returns undef.
 */
static SharedPtr<Value> mc_superclass(VM* vm, Value* self) {
  SharedValue  super = class_superclass(SELF(self));
  if (super.get()) {
    return super.get();
  } else {
    return new_undef_value();
  }
}

/**
 * $meta.bless(Any $data) : Object
 *
 * Create new instance with $data.
 */
static SharedPtr<Value> mc_bless(VM* vm, Value* self, Value* data) {
  return new ObjectValue(vm, self, data);
}

void tora::Init_MetaClass(VM* vm) {
  ClassBuilder builder(vm, SYMBOL_METACLASS_CLASS);
  builder.add_method("has_method", new CallbackFunction(mc_has_method));
  builder.add_method("get_method_list",
                    new CallbackFunction(mc_get_method_list));
  builder.add_method("name", new CallbackFunction(mc_name));
  builder.add_method("superclass", new CallbackFunction(mc_superclass));
  builder.add_method("bless", new CallbackFunction(mc_bless));
  vm->add_builtin_class(builder.value());
}
