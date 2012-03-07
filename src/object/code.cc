#include "../vm.h"
#include "../value/code.h"
#include "code.h"

using namespace tora;

/**
 * $code.package() : Str
 *
 * Get a declared package
 */
static SharedPtr<Value> code_package(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_CODE);
    return new StrValue(vm->symbol_table->id2name(self->upcast<CodeValue>()->package_id));
}

static SharedPtr<Value> code_name(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_CODE);
    return new StrValue(vm->symbol_table->id2name(self->upcast<CodeValue>()->func_name_id));
}

void tora::Init_Code(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package("Code");
    pkg->add_method(vm->symbol_table->get_id("package"), new CallbackFunction(code_package));
    pkg->add_method(vm->symbol_table->get_id("name"), new CallbackFunction(code_name));
}

