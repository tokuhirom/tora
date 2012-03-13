#include "dynaloader.h"
#include "../shared_ptr.h"
#include "../value.h"
#include "../vm.h"
#include "../package.h"

using namespace tora;

static SharedPtr<Value> dynaloader_load(VM *vm, Value *self, Value *filename_v, Value *endpoint_v) {
    SharedPtr<StrValue> filename_s = filename_v->to_s();
    SharedPtr<StrValue> endpoint_s = endpoint_v->to_s();
    vm->load_dynamic_library(filename_s->str_value, endpoint_s->str_value);
    return UndefValue::instance();
}

void tora::Init_DynaLoader(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("DynaLoader");
    pkg->add_method(vm->symbol_table->get_id("load"), new CallbackFunction(dynaloader_load));
}

