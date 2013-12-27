#include "../object.h"
#include "../shared_ptr.h"
#include "../inspector.h"
#include "../value.h"
#include "../value/object.h"
#include "../value/symbol.h"
#include "../value/class.h"
#include "../vm.h"
#include "../peek.h"
#include "../symbols.gen.h"
#include "../class_builder.h"

using namespace tora;

/**
 * class Object
 *
 * Object is a base class of all classes.
 * It's known as UNIVERSAL package in Perl5.
 */

/**
 * $object.tora() : String
 *
 * convert object to tora source.
 */
static SharedPtr<Value> object_tora(VM *vm, Value *self) {
  Inspector ins(vm);
  return new_str_value(ins.inspect(self));
}

/**
 * $object.meta() : MetaClass
 *
 * get a meta class.
 */
static SharedPtr<Value> object_meta(VM *vm, Value *self) {
  auto n = vm->get_builtin_class(SYMBOL_METACLASS_CLASS);
  if (self->value_type == VALUE_TYPE_SYMBOL) {
    MortalObjectValue o(
      vm,
      n.get(),
      vm->get_class(symbol_id(self)).get()
    );
    return o.get();
  } else if (self->value_type == VALUE_TYPE_OBJECT) {
    MortalObjectValue o(vm, n.get(), object_class(self));
    return o.get();
  } else if (self->value_type == VALUE_TYPE_CLASS) {
    MortalObjectValue o(vm, n.get(), self);
    return o.get();
  } else {
    MortalObjectValue o(
      vm,
      n.get(),
      vm->get_builtin_class(self->object_package_id()).get()
    );
    return o.get();
  }
}

/**
 * $object.isa($target) : Boolean
 *
 * This method returns true if $object is-a $target, false otherwise.
 */
static SharedPtr<Value> st_object_isa(VM *vm, Value *self, Value *target_v) {
  if (self->value_type == VALUE_TYPE_OBJECT) {
    StringImpl target = target_v->to_s();
    ID target_id = vm->symbol_table->get_id(target);
    return vm->to_bool(object_isa(self, target_id));
  } else {
    TODO();
  }
}

/**
 * $object.bless(Any $data) : Object
 *
 * Create new instance with $data.
 */
static SharedPtr<Value> mc_bless(VM *vm, Value *self, Value *data) {
  if (self->value_type == VALUE_TYPE_CLASS) {
    MortalObjectValue o(vm, self, data);
    return o.get();
  } else {
    throw new ExceptionValue("You cannot bless non-class value.");
  }
}

void tora::Init_Object(VM *vm) {
  ClassBuilder builder(vm, SYMBOL_OBJECT_CLASS);
  builder.add_method("tora", new CallbackFunction(object_tora));
  builder.add_method("meta", new CallbackFunction(object_meta));
  builder.add_method("isa", new CallbackFunction(st_object_isa));
  builder.add_method("bless", new CallbackFunction(mc_bless));
  vm->add_builtin_class(builder.value());
}
