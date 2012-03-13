#include "object.h"
#include "../shared_ptr.h"
#include "../inspector.h"
#include "../value.h"
#include "../package.h"
#include "../vm.h"

using namespace tora;

static SharedPtr<Value> object_tora(VM *vm, Value *self) {
    Inspector ins(vm);
    return new StrValue(ins.inspect(self));
}

void tora::Init_Object(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("Object");
    pkg->add_method(vm->symbol_table->get_id("tora"), new CallbackFunction(object_tora));
}

