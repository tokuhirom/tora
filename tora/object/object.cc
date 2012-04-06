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
    return new StrValue(ins.inspect(self));
}

/**
 * $object.meta() : MetaClass
 *
 * get a meta class.
 */
static SharedPtr<Value> object_meta(VM *vm, Value *self) {
    if (self->value_type == VALUE_TYPE_SYMBOL) {
        return vm->get_class(self->upcast<SymbolValue>()->id());
    } else if (self->value_type == VALUE_TYPE_OBJECT) {
        return self->upcast<ObjectValue>()->class_value();
    } else {
        return vm->get_builtin_class(self->object_package_id());
    }
}

/**
 * $object.isa($target) : Boolean
 *
 * This method returns true if $object is-a $target, false otherwise.
 */
static SharedPtr<Value> object_isa(VM *vm, Value *self, Value *target_v) {
    if (self->value_type == VALUE_TYPE_OBJECT) {
        SharedPtr<StrValue> target = target_v->to_s();
        ID target_id = vm->symbol_table->get_id(target->str_value());
        return BoolValue::instance(self->upcast<ObjectValue>()->isa(target_id));
    } else {
        TODO();
    }
}

void tora::Init_Object(VM *vm) {
    SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_OBJECT_CLASS);
    klass->add_method("tora", new CallbackFunction(object_tora));
    klass->add_method("meta", new CallbackFunction(object_meta));
    klass->add_method("isa",  new CallbackFunction(object_isa));
    vm->add_builtin_class(klass);
}

