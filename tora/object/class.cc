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
 * class Class
 *
 * This is a Class class.
 */

inline static ClassValue* SELF(Value *self) {
    return self->upcast<ClassValue>();
}

/**
 * $meta.name() : String
 *
 * Get a name of class.
 */
static SharedPtr<Value> Class_name(VM * vm, Value* self) {
    return new_str_value(SELF(self)->name());
}

void tora::Init_Class(VM* vm) {
    SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_CLASS_CLASS);
    klass->add_method("name", new CallbackFunction(Class_name));
    vm->add_builtin_class(klass);
}

