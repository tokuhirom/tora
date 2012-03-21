#include <stdlib.h>

#include "../object.h"
#include "../vm.h"
#include "../package.h"
#include "../value/object.h"

using namespace tora;

/**
 * $caller.code() : Code
 *
 * Return the caller code object.
 */
static SharedPtr<Value> caller_code(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    return self->upcast<ObjectValue>()->data();
}

void tora::Init_Caller(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("Caller");
    pkg->add_method(vm->symbol_table->get_id("code"), new CallbackFunction(caller_code));
}

