#include "object.h"
#include "../shared_ptr.h"
#include "../inspector.h"
#include "../value.h"
#include "../value/object.h"
#include "../package.h"
#include "../vm.h"
#include "../symbols.gen.h"

using namespace tora;

/**
 * convert object to tora source.
 */
static SharedPtr<Value> object_tora(VM *vm, Value *self) {
    Inspector ins(vm);
    return new StrValue(ins.inspect(self));
}

/**
 * $object.meta();
 *
 * get a meta class.
 */
static SharedPtr<Value> object_meta(VM *vm, Value *self) {
    return new ObjectValue(vm, SYMBOL_METACLASS_CLASS, self);
}

void tora::Init_Object(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("Object");
    pkg->add_method(vm->symbol_table->get_id("tora"), new CallbackFunction(object_tora));
    pkg->add_method(vm->symbol_table->get_id("meta"), new CallbackFunction(object_meta));
}

