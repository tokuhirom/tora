#include "../object.h"
#include "../vm.h"
#include "../value/array.h"
#include "../value/object.h"
#include "../value/code.h"
#include "../value/class.h"
#include "../peek.h"
#include "../symbols.gen.h"

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

/**
 * $meta.has_method(Str $name) : Boolean
 *
 * Returns a boolean indicating whether or not the class defines the named method.
 * It does not include methods inherited from parent classes.
 */
static SharedPtr<Value> mc_has_method(VM * vm, Value* self, Value * methname_v) {
    SharedPtr<StrValue> methname = methname_v->to_s();
    bool ret = self->upcast<ClassValue>()->has_method(vm->symbol_table->get_id(methname->str_value()));
    return BoolValue::instance(ret);
}

/**
 * $meta.get_method_list() : Array[Str]
 *
 * Get a method list defined in package.
 */
static SharedPtr<Value> mc_get_method_list(VM * vm, Value* self) {
    return self->upcast<ClassValue>()->get_method_list();
}


/**
 * $meta.name() : String
 *
 * Get a name of class.
 */
static SharedPtr<Value> mc_name(VM * vm, Value* self) {
    return new StrValue(self->upcast<ClassValue>()->name());
}

/**
 * $meta.superclass() : String
 *
 * This method returns superclass in string.
 * If the class does not have a superclass, it returns undef.
 */
static SharedPtr<Value> mc_superclass(VM * vm, Value* self) {
    const SharedPtr<ClassValue> &super = self->upcast<ClassValue>()->superclass();
    if (super.get()) {
        return super;
    } else {
        return UndefValue::instance();
    }
}

void tora::Init_Class(VM* vm) {
    SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_CLASS_CLASS);
    klass->add_method("has_method", new CallbackFunction(mc_has_method));
    klass->add_method("get_method_list", new CallbackFunction(mc_get_method_list));
    klass->add_method("name", new CallbackFunction(mc_name));
    klass->add_method("superclass", new CallbackFunction(mc_superclass));
    vm->add_builtin_class(klass);
}

