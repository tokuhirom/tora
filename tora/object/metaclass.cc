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

inline static ClassValue* SELF(Value *self) {
    const SharedPtr<Value> &dat = self->upcast<ObjectValue>()->data();
    return dat->upcast<ClassValue>();
}

/**
 * $meta.has_method(Str $name) : Boolean
 *
 * Returns a boolean indicating whether or not the class defines the named method.
 * It does not include methods inherited from parent classes.
 */
static SharedPtr<Value> mc_has_method(VM * vm, Value* self, Value * methname_v) {
    std::string methname = methname_v->to_s();
    bool ret = SELF(self)->has_method(vm->symbol_table->get_id(methname));
    return vm->to_bool(ret);
}

/**
 * $meta.get_method_list() : Array[Str]
 *
 * Get a method list defined in package.
 */
static SharedPtr<Value> mc_get_method_list(VM * vm, Value* self) {
    return SELF(self)->get_method_list();
}


/**
 * $meta.name() : String
 *
 * Get a name of class.
 */
static SharedPtr<Value> mc_name(VM * vm, Value* self) {
    return new_str_value(SELF(self)->name());
}

/**
 * $meta.superclass() : String
 *
 * This method returns superclass in string.
 * If the class does not have a superclass, it returns undef.
 */
static SharedPtr<Value> mc_superclass(VM * vm, Value* self) {
    const SharedPtr<ClassValue> &super = SELF(self)->superclass();
    if (super.get()) {
        return super;
    } else {
        return new_undef_value();
    }
}

/**
 * $meta.bless(Any $data) : Object
 *
 * Create new instance with $data.
 */
static SharedPtr<Value> mc_bless(VM * vm, Value* self, Value *data) {
    return new ObjectValue(vm, self->upcast<ClassValue>(), data);
}

void tora::Init_MetaClass(VM* vm) {
    SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_METACLASS_CLASS);
    klass->add_method("has_method", new CallbackFunction(mc_has_method));
    klass->add_method("get_method_list", new CallbackFunction(mc_get_method_list));
    klass->add_method("name", new CallbackFunction(mc_name));
    klass->add_method("superclass", new CallbackFunction(mc_superclass));
    klass->add_method("bless", new CallbackFunction(mc_bless));
    vm->add_builtin_class(klass);
}

